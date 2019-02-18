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

//------------------------------------------------------------------------------

namespace NOscillator
{
  struct Model
  {
    struct Decay: Property<float>
    {
      Decay()
      {
        setValue(0.22f);
        setRange(0.01f, 5.f);
      }
    };

    using RootNote = RootNoteProperty;
    using Scale = ScaleProperty;

    using Properties = PropertySet<Decay, RootNote, Scale>;
  };

  class Applet : public ArticCircleApplet<Model> {
  public:

      Applet()
      {
        setName("Lil.Osc");

        root_ = 0;
        quantizer_.Init();
        lastNote_ = 0;
        eg_.init();

        setCallback<Model::Decay>([this](const auto& decay){
          eg_.setCoefficients(calcSlewCoeff(16), calcSlewCoeff(kSampleRate * decay));
        });

        setCallback<Model::Scale>([this](const auto& scale){
          quantizer_.Configure(OC::Scales::GetScale(scale), 0xffff);
        });

        bind<Model::RootNote>(root_);

        osc_.setTicker([](const sample_t& phase)
        {
          return Sine(phase);
        });
      }

      void reset() final
      {
        osc_.reset(kSampleRate);
      }

      void tick() final
      {
        const int32_t pitch = In(0);
        const int32_t quantized = quantizer_.Process(pitch, root_ << 7, 0);
        lastNote_ = MIDIQuantizer::NoteNumber(quantized) -24;
        const float frequency = midiNoteToFrequency(lastNote_);
        osc_.setFrequency(frequency);
        Out(0, float(osc_.tick() * eg_.tick(Gate(0))) * HEMISPHERE_3V_CV);
      }

  	/* Draw the screen */
      void drawApplet() final
      {
        gfxSkyline();
          // Add other view code as private methods
      //  gfxPrint(21, 15, 10.f * decay_);
        ArticCircleApplet<Model>::drawApplet();

        if (eg_.value() > sample_t(1e-4))
        {
          gfxPrint(38, 50, midi_note_numbers[lastNote_]);
        }
      }

  protected:
      // /* Set help text. Each help section can have up to 18 characters. Be concise! */
      // void SetHelp() {
      //     //                               "------------------" <-- Size Guide
      //     help[HEMISPHERE_HELP_DIGITALS] = "Digital in help";
      //     help[HEMISPHERE_HELP_CVS]      = "CV in help";
      //     help[HEMISPHERE_HELP_OUTS]     = "Out help";
      //     help[HEMISPHERE_HELP_ENCODER]  = "123456789012345678";
      //     //                               "------------------" <-- Size Guide
      // }

  private:
    ADEnvelope<sample_t> eg_;
    Oscillator<sample_t> osc_;
    braids::Quantizer quantizer_;
    int32_t lastNote_;
    int root_;
  };

  Applet instance_[2];
} // NOscillator

////////////////////////////////////////////////////////////////////////////////
void Oscillator_Start(bool hemisphere) {NOscillator::instance_[hemisphere].BaseStart(hemisphere);}
void Oscillator_Controller(bool hemisphere, bool forwarding) {NOscillator::instance_[hemisphere].BaseController(forwarding);}
void Oscillator_View(bool hemisphere) {NOscillator::instance_[hemisphere].BaseView();}
void Oscillator_OnButtonPress(bool hemisphere) {NOscillator::instance_[hemisphere].OnButtonPress();}
void Oscillator_OnEncoderMove(bool hemisphere, int direction) {NOscillator::instance_[hemisphere].OnEncoderMove(direction);}
void Oscillator_ToggleHelpScreen(bool hemisphere) {NOscillator::instance_[hemisphere].HelpScreen();}
uint32_t Oscillator_OnDataRequest(bool hemisphere) {return NOscillator::instance_[hemisphere].OnDataRequest();}
void Oscillator_OnDataReceive(bool hemisphere, uint32_t data) {NOscillator::instance_[hemisphere].OnDataReceive(data);}
