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
    enum class Waveforms
    {
      Sine,
      QuadraticSine,
      Square,
      BRect,
      COUNT
    };

    struct Decay: Property<float>
    {
      Decay()
      {
        setValue(0.22f);
        setRange(0.01f, 5.f, 0.02f);
        setExponentialScaling(3.f);
      }

      using ValueConverter = ExponentialValueConverter;
    };

    struct Waveform: Property<Waveforms>
    {
      Waveform()
      {
        setValue(Waveforms::Sine);
        setEnumStrings({"Sine", "QSine", "Square", "BSquare"});
      }
    };

    using RootNote = RootNoteProperty;
    using Scale = ScaleProperty;
    using Octave = OctaveProperty;

    using Properties = PropertySet<RootNote, Scale, Octave, Waveform, Decay>;
  };

  class Applet : public ArticCircleApplet<Model> {
  public:

      Applet()
      {
        setName("Lil.Osc");

        root_ = 0;
        quantizer_.Init();
        lastNote_ = 0;
        //eg_.init();
        leg_.init();

        setCallback<Model::Decay>([this](const auto& decay){
          //eg_.setCoefficients(calcSlewCoeff(16), calcSlewCoeff(kSampleRate * decay));
          leg_.setCoefficients(16, uint32_t(kSampleRate * decay));
        });

        setCallback<Model::Scale>([this](const auto& scale){
          quantizer_.Configure(OC::Scales::GetScale(scale), 0xffff);
        });

        bind<Model::RootNote>(root_);

        setCallback<Model::Waveform>([this](const auto& waveform){
          switch(waveform)
          {
            case Model::Waveforms::Sine:
              osc_.setTicker([](const sample_t& phase, const sample_t& /*phaseInc*/)
              {
                return Sine(phase);
              });
              break;
            case Model::Waveforms::QuadraticSine:
              osc_.setTicker([](const sample_t& phase, const sample_t& /*phaseInc*/)
              {
                return quadraticSine(phase);
              });
              break;

            case Model::Waveforms::Square:
              osc_.setTicker([](const sample_t& phase, const sample_t& /*phaseInc*/)
              {
                return phase < sample_t(0.5) ? sample_t(-1) : sample_t(1);
              });
              break;

            case Model::Waveforms::BRect:
              osc_.setTicker([](const sample_t& phase, const sample_t& phaseInc)
              {
                return rectPolyBlep(phase, phaseInc);
              });
              break;

            default:
              break;
          }
        });

        setCallback<Model::Octave>([this](const auto& o){
          octave_ = o;
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
        lastNote_ = clamp(MIDIQuantizer::NoteNumber(quantized) -24 + octave_ * 12, 0 , 127);
        const float frequency = midiNoteToFrequency(lastNote_) ;
        osc_.setFrequency(frequency);
        Out(0, float(osc_.tick() * leg_.tick(Gate(0))) * HEMISPHERE_3V_CV);
      }

  	/* Draw the screen */
      void drawApplet() final
      {
        gfxSkyline();
          // Add other view code as private methods
      //  gfxPrint(21, 15, 10.f * decay_);
        ArticCircleApplet<Model>::drawApplet();

        if (leg_.value() > sample_t(1e-4))
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
//    ADEnvelope<sample_t> eg_;
    LinearADEnvelope<sample_t> leg_;
    Oscillator<sample_t> osc_;
    braids::Quantizer quantizer_;
    int32_t lastNote_;
    int root_;
    int octave_;
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
