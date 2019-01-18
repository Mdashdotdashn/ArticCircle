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

#include "src/nostromo.h"

#include "braids_quantizer.h"
#include "braids_quantizer_scales.h"
#include "OC_scales.h"

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
      eg_.init(sample_t(0));
    }

	/* Run during the interrupt service routine, 16667 times per second */
    void Controller() {
      if (Clock(0, K(PhysicalOnly)))
      {
        eg_.ramp(sample_t(1),sample_t(0), 16667);
      }

      if (Changed(0))
      {
        const int32_t pitch = In(0);
        const int32_t quantized = quantizer_.Process(pitch, root_ << 7, 0);
        const uint8_t midiNote = MIDIQuantizer::NoteNumber(quantized) -24;
        const float frequency = midiNoteToFrequency(midiNote);
        phaseIncrease_ = sample_t(frequency / 16667.f);        
      }
      phase_ = sample_t::frac(phase_ + phaseIncrease_);
      Out(0, float(Sine(phase_) * eg_.tick()) * HEMISPHERE_3V_CV);
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
