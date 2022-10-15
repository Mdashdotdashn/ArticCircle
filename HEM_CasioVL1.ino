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

namespace NCasioVL1
{
  struct Model
  {
    enum class Sequences
    {
      Rock1,
      Rock2,
      Sync1,
      Funk1,
      Sync2,
      COUNT
    };

    struct Sequence: Property<Sequences>
    {
      Sequence()
      {
        setValue(Sequences::Rock1);
        setEnumStrings({"Rock 1", "Rock 2", "Sync 1", "Funk 1", "Sync 2"});
      }
    };

    using Properties = PropertySet<Sequence>;
  };

  class Applet : public ArticCircleApplet<Model>
  {
    enum Steps
    {
      rest = 0,
      low = 1,
      high = 2,
      noise = 3
    };

    constexpr static int8_t kSequenceSize = 16;
    constexpr static int8_t kSequenceCount = 5;
    Steps sequences_[kSequenceCount][16] = {
      { low, rest, high, rest, noise, rest, noise, rest, low, rest, high, rest, noise, rest, high, rest},
      { low, rest, high, rest, noise, rest, high, low, low, rest, low, rest, noise, rest, high, rest},
      { low, rest, high, rest, rest, high, rest, rest, low, rest, high, rest, noise, rest, high, rest},
      { low, rest, low, rest, noise, rest, rest, high, noise, rest, low, rest, high, rest, low, rest},
      { low, rest, high, low, rest, high, rest, low, noise, rest, rest, high, rest, rest, noise, rest}
    };

  public:
    Applet()
    {
      // Maximum 9 characters
      //       123456789
      setName("Casio-VL1");

      setCallback<Model::Sequence>([this](const auto& v) {
        sequence_ = int(v);
      });
    }

    virtual void reset() final
    {
      position_ = 0;
      updateSound();
      eg_.init();
      phasor_.reset(kSampleRate);
    }

    void nextStep()
    {
      const auto step = sequences_[sequence_][position_];
      if (step_ != step)
      {
        step_ = step;
        if (step_ != rest)
        {
          gate_ = true;
          updateSound();
        }
      }
      position_ = (position_ + 1) % kSequenceSize;
    }

    void updateSound()
    {
      static constexpr float attackCoeff = calcSlewCoeff(16);
      static constexpr float decayLow = calcSlewCoeff(uint32_t(0.155f * kSampleRate));
      static constexpr float decayHigh = calcSlewCoeff(uint32_t(0.048f * kSampleRate));
      static constexpr float decayNoise = calcSlewCoeff(uint32_t(0.504f * kSampleRate));

      float decayCoeff = 0.f;
      float frequency = 0.f;

      switch(step_)
      {
        case low:
          decayCoeff = decayLow;
          frequency = 783.99f; // G4
        break;
        case high:
          decayCoeff = decayHigh;
          frequency = 1567.98f; // G5
        break;
        case noise:
          decayCoeff = decayNoise;
          frequency = 100.f; // Dummy but we don't want to derail the phasor
        break;
        default:
          return;
      }
      phasor_.setFrequency(frequency);
      eg_.setCoeffs(attackCoeff, decayCoeff);
    }

    virtual void tick() final
    {
      gate_ = false;

      if (flankUp(1))
      {
          position_ = 0;
      }
      if (Clock(0))
      {
        nextStep();
      }

      sizer_.feed(gate_);
      const auto phase = phasor_.tick();
      const auto phaseInc = phasor_.phaseInc();

      // compute sample according to sound mode
      const auto v = (step_ == noise) ? rand_.tick() : (rectPolyBlep(phase, phaseInc) + sample_t(1)) * sample_t(0.5);
      const auto value = clamp(v * eg_.tick(gate_), sample_t(-1), sample_t(1));
      Out(0, float(value) * HEMISPHERE_MAX_CV);
    }

    void drawApplet() final
    {
      ArticCircleApplet<NCasioVL1::Model>::drawApplet();

      const auto size = sizer_.updateSize();
      if ((size > 0) && (step_ != rest))
      {
        gfxRect(2 + 21 * (step_ -1), 38, 20, 10);
      }
    }

  private:
    //     return v * eg_.tick(gate);
    Random<sample_t> rand_;
    Phasor<sample_t> phasor_;
    ADEnvelope<sample_t> eg_;
    int position_ = 0;
    int sequence_ = 0;
    bool gate_;
    Steps step_ = Steps::rest;

    TriggerSizer<16, 24> sizer_;
  };

  Applet instance_[2];

} // NCasioVL1

void CasioVL1_Start(bool hemisphere) {NCasioVL1::instance_[hemisphere].BaseStart(hemisphere);}
void CasioVL1_Controller(bool hemisphere, bool forwarding) {NCasioVL1::instance_[hemisphere].BaseController(forwarding);}
void CasioVL1_View(bool hemisphere) {NCasioVL1::instance_[hemisphere].BaseView();}
void CasioVL1_OnButtonPress(bool hemisphere) {NCasioVL1::instance_[hemisphere].OnButtonPress();}
void CasioVL1_OnEncoderMove(bool hemisphere, int direction) {NCasioVL1::instance_[hemisphere].OnEncoderMove(direction);}
void CasioVL1_ToggleHelpScreen(bool hemisphere) {NCasioVL1::instance_[hemisphere].HelpScreen();}
uint32_t CasioVL1_OnDataRequest(bool hemisphere) {return NCasioVL1::instance_[hemisphere].OnDataRequest();}
void CasioVL1_OnDataReceive(bool hemisphere, uint32_t data) {NCasioVL1::instance_[hemisphere].OnDataReceive(data);}
