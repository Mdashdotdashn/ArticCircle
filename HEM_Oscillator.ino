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
      TanhSine,
      Square,
      BRect,
      SharkTooth,
      Random,
      COUNT
    };

    enum class Envelopes
    {
      Exponential,
      Linear,
      COUNT
    };

    struct Decay: Property<float>
    {
      Decay()
      {
        setValue(0.22f);
        setRange(0.01f, 5.f, 0.005f);
        setExponentialScaling(3.f);
      }

      using ValueConverter = ExponentialValueConverter;
      using StringConverter = TimeStringConverter;
    };

    struct Waveform: Property<Waveforms>
    {
      Waveform()
      {
        setValue(Waveforms::Sine);
        setEnumStrings({"Sine", "QSine", "Tanh", "Square", "BSquare", "Shark", "Random"});
      }
    };

    struct Envelope: Property<Envelopes>
    {
      Envelope()
      {
        setValue(Envelopes::Exponential);
        setEnumStrings({"Exp", "Lin"});
      }
    };

    using RootNote = RootNoteProperty;
    using Scale = ScaleProperty;
    using Octave = OctaveProperty;

    using Properties = PropertySet<RootNote, Scale, Octave, Waveform, Envelope, Decay>;
  };

  class SwitchableEG
  {
  public:
    using Envelopes = Model::Envelopes;

    SwitchableEG()
    : envelope_(Envelopes::Exponential)
    {}

    void init()
    {
      eg_.init();
      leg_.init();
    }

    void setMode(const Envelopes envelope)
    {
      envelope_ = envelope;
      eg_.reset();
      leg_.reset();
    }

    void setDecay(float decayTime)
    {
      eg_.setSlopes(16, uint32_t(kSampleRate * decayTime));
      leg_.setSlopes(16, uint32_t(kSampleRate * decayTime));
    }

    sample_t tick(const bool gate)
    {
      return envelope_ == Envelopes::Exponential ? eg_.tick(gate) : leg_.tick(gate);
    }

    sample_t value()
    {
      return envelope_ == Envelopes::Exponential ? eg_.value() : leg_.value();
    }

  private:
    Envelopes envelope_;
    ADEnvelope<sample_t> eg_;
    LinearADEnvelope<sample_t> leg_;
  };

  //-------------------------------------------------------------------------------------------------------

  class Applet : public ArticCircleApplet<Model> {
  public:

      Applet()
      {
        setName("Lil.Osc");

        root_ = 0;
        quantizer_.Init();
        lastNote_ = 0;

        random::uniform_distribution(random::get_seed());
        eg_.init();

        setCallback<Model::Decay>([this](const auto& decay){
          eg_.setDecay(decay);
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

              case Model::Waveforms::TanhSine:
                osc_.setTicker([](const sample_t& phase, const sample_t& /*phaseInc*/)
                {
                  return tanh(phase);
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
                return (rectPolyBlep(phase, phaseInc) + sample_t(1)) * sample_t(0.5);
              });
              break;

            case Model::Waveforms::SharkTooth:
              osc_.setTicker([this](const sample_t& phase, const sample_t& phaseInc)
              {
                return sharkShape_.tick(phase, phaseInc, sample_t(0.7));
              });
              break;

            case Model::Waveforms::Random:
              osc_.setTicker([](const sample_t& /*phase*/, const sample_t& /*phaseInc*/)
              {
                return rand<sample_t>();
              });
              break;

            default:
              break;
          }
        });

        setCallback<Model::Octave>([this](const auto& o){
          octave_ = o;
        });

        setCallback<Model::Envelope>([this](const auto& o){
          eg_.setMode(o);
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
        const auto osc = osc_.tick();
        Out(0, float(osc * eg_.tick(Gate(0))) * HEMISPHERE_3V_CV);
        Out(1, float(osc) * HEMISPHERE_3V_CV);
      }

  	/* Draw the screen */
      void drawApplet() final
      {
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
    Oscillator<sample_t> osc_;
    SharkToothShape<sample_t> sharkShape_;
    SwitchableEG eg_;
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
