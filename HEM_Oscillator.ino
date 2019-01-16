// Copyright (c) 2018, Marc Nostromo
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "nostromo/dsp.h"
#include "nostromo/FixedFP.h"

#include "braids_quantizer.h"
#include "braids_quantizer_scales.h"
#include "OC_scales.h"

static const float sMidiNoteToFrequency[128] = 
{
16.3515978313f,17.3239144361f,18.3540479948f,19.4454364826f,
20.6017223071f,21.8267644646f,23.1246514195f,24.4997147489f,
25.9565435987f,27.5000000000f,29.1352350949f,30.8677063285f,
32.7031956626f,34.6478288721f,36.7080959897f,38.8908729653f,
41.2034446141f,43.6535289291f,46.2493028390f,48.9994294977f,
51.9130871975f,55.0000000000f,58.2704701898f,61.7354126570f,
65.4063913251f,69.2956577442f,73.4161919794f,77.7817459305f,
82.4068892282f,87.3070578583f,92.4986056779f,97.9988589954f,
103.8261743950f,110.0000000000f,116.5409403795f,123.4708253140f,
130.8127826503f,138.5913154884f,146.8323839587f,155.5634918610f,
164.8137784564f,174.6141157165f,184.9972113558f,195.9977179909f,
207.6523487900f,220.0000000000f,233.0818807590f,246.9416506281f,
261.6255653006f,277.1826309769f,293.6647679174f,311.1269837221f,
329.6275569129f,349.2282314330f,369.9944227116f,391.9954359818f,
415.3046975799f,440.0000000000f,466.1637615181f,493.8833012561f,
523.2511306012f,554.3652619537f,587.3295358348f,622.2539674442f,
659.2551138257f,698.4564628660f,739.9888454233f,783.9908719635f,
830.6093951599f,880.0000000000f,932.3275230362f,987.7666025123f,
1046.5022612024f,1108.7305239075f,1174.6590716696f,1244.5079348883f,
1318.5102276515f,1396.9129257320f,1479.9776908465f,1567.9817439270f,
1661.2187903198f,1760.0000000000f,1864.6550460724f,1975.5332050245f,
2093.0045224048f,2217.4610478150f,2349.3181433393f,2489.0158697767f,
2637.0204553030f,2793.8258514640f,2959.9553816931f,3135.9634878540f,
3322.4375806396f,3520.0000000000f,3729.3100921447f,3951.0664100490f,
4186.0090448096f,4434.9220956300f,4698.6362866785f,4978.0317395533f,
5274.0409106059f,5587.6517029281f,5919.9107633862f,6271.9269757080f,
6644.8751612792f,7040.0000000000f,7458.6201842895f,7902.1328200980f,
8372.0180896192f,8869.8441912600f,9397.2725733571f,9956.0634791066f,
10548.0818212119f,11175.3034058562f,11839.8215267724f,12543.8539514160f,
13289.7503225583f,14080.0000000001f,14917.2403685790f,15804.2656401961f,
16744.0361792384f,17739.6883825199f,18794.5451467142f,19912.1269582133f,
21096.1636424238f,22350.6068117124f,23679.6430535447f,25087.7079028321f
};

  
template <typename T>
class ExponentialSegment
{
public:

  ExponentialSegment();
  ~ExponentialSegment();

  void Setup(
             const T& from,
             const T& to,
             const uint32_t timeInTriggerStep);

  void UpdateSpeed(const uint32_t timeInTriggerStep);

  T Tick();
  bool EndOfSegment();

  T Value()
  {
    return current_;
  }

private:

  void updateCurrent();

private:
  const static float kNoiseFloor;
  T from_;
  T to_;

  bool endOfSegment_;

  T speed_;
  T ramp_;

  T current_;

  static const T ONE_;
  static const T ZERO_;
};

template <typename T> const T ExponentialSegment<T>::ZERO_ = T(0);
template <typename T> const T ExponentialSegment<T>::ONE_ = T(1);

template <typename T>
const float ExponentialSegment<T>::kNoiseFloor = 1e-4f; // -80db

template <typename T>
ExponentialSegment<T>::ExponentialSegment()
:endOfSegment_(true)
,speed_(ZERO_)
,ramp_(ZERO_)
{
}


//-----------------------------------------------------------

template <typename T>
ExponentialSegment<T>::~ExponentialSegment()
{
}

//-----------------------------------------------------------

template <typename T>
void ExponentialSegment<T>::Setup(
                             const T &from,
                             const T &to,
                             const uint32_t timeInTriggerStep)
{
  from_ = from;
  to_ = to;
  
  endOfSegment_ = (timeInTriggerStep == 0)  ;
  
  if (!endOfSegment_)
  {
    speed_ = T(powf(kNoiseFloor, 1.f/float(timeInTriggerStep)));
  }
  ramp_ = endOfSegment_ ? ZERO_ : ONE_;
  updateCurrent();
}


//-----------------------------------------------------------

template <typename T>
void ExponentialSegment<T>::UpdateSpeed(const uint32_t timeInTriggerStep)
{
  speed_ = T(powf(kNoiseFloor, 1.f/float(timeInTriggerStep)));  
}


//-----------------------------------------------------------

template <typename T>
T ExponentialSegment<T>::Tick()
{  
  ramp_ *= speed_;
  endOfSegment_ = (ramp_ <= T(kNoiseFloor));
  updateCurrent();
  return current_;
}


template <typename T>
void ExponentialSegment<T>::updateCurrent()
{
  current_ = from_ + (to_ - from_) * ramp_;
}

//-----------------------------------------------------------

template <typename T>
bool ExponentialSegment<T>::EndOfSegment()
{
  return endOfSegment_ ;
}

class Oscillator : public HemisphereApplet {
public:

    using sample_t = FixedFP<int32_t, 27>;

    const char* applet_name() { // Maximum 10 characters
        return "Oscillator";
    }

	/* Run when the Applet is selected */
    void Start() {
      scale_ = 6; // Major
      root_ = 0;
      phaseIncrease_ = sample_t(80.f / 16667.f);
      phase_ = sample_t(0);
      quantizer_.Init();
      quantizer_.Configure(OC::Scales::GetScale(scale_), 0xffff);
      lastNote_ = 0;
      eg_.Setup(sample_t(1),sample_t(0), 16667);
    }

	/* Run during the interrupt service routine, 16667 times per second */
    void Controller() {
      if (Clock(0))
      {
        eg_.Setup(sample_t(1),sample_t(0), 16667);
      }

      if (Changed(0))
      {
        const int32_t pitch = In(0);
        const int32_t quantized = quantizer_.Process(pitch, root_ << 7, 0);
        const uint8_t midiNote = MIDIQuantizer::NoteNumber(quantized) -24;
        const float frequency = sMidiNoteToFrequency[midiNote];
        phaseIncrease_ = sample_t(frequency / 16667.f);        
      }
      phase_ = sample_t::frac(phase_ + phaseIncrease_);
      Out(0, float(Sine(phase_) * (sample_t(1) - eg_.Tick())) * HEMISPHERE_3V_CV);
    }

	/* Draw the screen */
    void View() {
        gfxHeader(applet_name());
        gfxSkyline();
        // Add other view code as private methods
    }

	/* Called when the encoder button for this hemisphere is pressed */
    void OnButtonPress() {
    }

	/* Called when the encoder for this hemisphere is rotated
	 * direction 1 is clockwise
	 * direction -1 is counterclockwise
	 */
    void OnEncoderMove(int direction) {
    }

    /* Each applet may save up to 32 bits of data. When data is requested from
     * the manager, OnDataRequest() packs it up (see HemisphereApplet::Pack()) and
     * returns it.
     */
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        // example: pack property_name at bit 0, with size of 8 bits
        // Pack(data, PackLocation {0,8}, property_name);
        return data;
    }

    /* When the applet is restored (from power-down state, etc.), the manager may
     * send data to the applet via OnDataReceive(). The applet should take the data
     * and unpack it (see HemisphereApplet::Unpack()) into zero or more of the applet's
     * properties.
     */
    void OnDataReceive(uint32_t data) {
        // example: unpack value at bit 0 with size of 8 bits to property_name
        // property_name = Unpack(data, PackLocation {0,8});
    }

protected:
    /* Set help text. Each help section can have up to 18 characters. Be concise! */
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "Digital in help";
        help[HEMISPHERE_HELP_CVS]      = "CV in help";
        help[HEMISPHERE_HELP_OUTS]     = "Out help";
        help[HEMISPHERE_HELP_ENCODER]  = "123456789012345678";
        //                               "------------------" <-- Size Guide
    }

private:
  ExponentialSegment<sample_t> eg_;
  sample_t phaseIncrease_;
  sample_t phase_;
  braids::Quantizer quantizer_;
  int32_t lastNote_;
  int scale_;
  int root_;
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to Oscillator,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
Oscillator Oscillator_instance[2];

void Oscillator_Start(bool hemisphere) {Oscillator_instance[hemisphere].BaseStart(hemisphere);}
void Oscillator_Controller(bool hemisphere, bool forwarding) {Oscillator_instance[hemisphere].BaseController(forwarding);}
void Oscillator_View(bool hemisphere) {Oscillator_instance[hemisphere].BaseView();}
void Oscillator_OnButtonPress(bool hemisphere) {Oscillator_instance[hemisphere].OnButtonPress();}
void Oscillator_OnEncoderMove(bool hemisphere, int direction) {Oscillator_instance[hemisphere].OnEncoderMove(direction);}
void Oscillator_ToggleHelpScreen(bool hemisphere) {Oscillator_instance[hemisphere].HelpScreen();}
uint32_t Oscillator_OnDataRequest(bool hemisphere) {return Oscillator_instance[hemisphere].OnDataRequest();}
void Oscillator_OnDataReceive(bool hemisphere, uint32_t data) {Oscillator_instance[hemisphere].OnDataReceive(data);}
