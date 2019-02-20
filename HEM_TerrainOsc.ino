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
#include "src/nostromo/terrain.h"

namespace NTerrainOsc
{
  struct Model
  {
    using RootNote = RootNoteProperty;
    using Scale = ScaleProperty;

    struct Harmonics: Property<float>
    {
      Harmonics()
      {
        setLabel("h");
        setRange(1.f, 7.f);
        setValue(1.f);
      }
    };

    struct Motion: Property<float>
    {
      Motion()
      {
        setLabel("m");
        setRange(0.f, 1.f, 0.01f);
        setValue(0.f);
      }
    };

    struct Octave: Property<int>
    {
      Octave()
      {
        setLabel("o");
        setRange(-12,3);
        setValue(-1);
      }
    };

    using Properties = PropertySet<RootNote, Scale, Octave, Harmonics, Motion>;
  };


  class Applet : public ArticCircleApplet<Model> {
  public:
    Applet()
    {
      // Maximum 9 characters
      //       123456789
      setName("Terrain");

      root_ = 0;
      quantizer_.Init();
      lastNote_ = 0;

      bind<Model::RootNote>(root_);

      setCallback<Model::Scale>([this](const auto& scale){
        quantizer_.Configure(OC::Scales::GetScale(scale), 0xffff);
      });

      setCallback<Model::Harmonics>([this](const auto& h){
        radius_ = h/4.;
      });

      setCallback<Model::Motion>([this](const auto& m){
        motion_.setFrequency(lerp(0., 0.1, m));
      });

      setCallback<Model::Octave>([this](const auto& o){
        freqMult_ = powf(2., o);
      });
    }

    virtual void reset() final
    {
      phasor_.reset(kSampleRate);
      motion_.reset(kSampleRate);
    }

    virtual void tick() final
    {
      const auto positionOffset = motion_.tick() * sample_t(8);
      const auto harmonicOffset = sample_t::fromRatio(DetentedIn(1), HEMISPHERE_MAX_CV) * sample_t(5);
      oscillator_.setParameters(radius_ + float(harmonicOffset), position_ + float(positionOffset), 0., 0.);

      const int32_t pitch = In(0);
      const int32_t quantized = quantizer_.Process(pitch, root_ << 7, 0);
      lastNote_ = MIDIQuantizer::NoteNumber(quantized) -24;
      const float frequency = midiNoteToFrequency(lastNote_);
      phasor_.setFrequency(frequency * freqMult_);
      auto value = oscillator_.tick(phasor_.tick());
      Out(0, float(value) * HEMISPHERE_3V_CV);
    }

    void drawApplet() final
    {
      ArticCircleApplet<NTerrainOsc::Model>::drawApplet();
    }

  private:
    Phasor<sample_t> phasor_;
    Phasor<sample_t> motion_;
    terrain::Oscillator<sample_t> oscillator_;
    braids::Quantizer quantizer_;
    float freqMult_;
    float radius_;
    float position_;
    int32_t lastNote_;
    int root_;
  };

  Applet instance_[2];

} // NTerrainOsc

void TerrainOsc_Start(bool hemisphere) {NTerrainOsc::instance_[hemisphere].BaseStart(hemisphere);}
void TerrainOsc_Controller(bool hemisphere, bool forwarding) {NTerrainOsc::instance_[hemisphere].BaseController(forwarding);}
void TerrainOsc_View(bool hemisphere) {NTerrainOsc::instance_[hemisphere].BaseView();}
void TerrainOsc_OnButtonPress(bool hemisphere) {NTerrainOsc::instance_[hemisphere].OnButtonPress();}
void TerrainOsc_OnEncoderMove(bool hemisphere, int direction) {NTerrainOsc::instance_[hemisphere].OnEncoderMove(direction);}
void TerrainOsc_ToggleHelpScreen(bool hemisphere) {NTerrainOsc::instance_[hemisphere].HelpScreen();}
uint32_t TerrainOsc_OnDataRequest(bool hemisphere) {return NTerrainOsc::instance_[hemisphere].OnDataRequest();}
void TerrainOsc_OnDataReceive(bool hemisphere, uint32_t data) {NTerrainOsc::instance_[hemisphere].OnDataReceive(data);}
