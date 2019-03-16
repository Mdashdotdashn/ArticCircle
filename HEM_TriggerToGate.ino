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

namespace NTriggerToGate
{
  struct Model
  {
    struct Size: Property<int>
    {
      Size()
      {
        setRange(2,24);
      }
    };

    struct PulseWidth: PercentageProperty
    {
      PulseWidth()
      {
        setValue(0.33);
      }
    };

    struct SizeL: Size {
      SizeL()
      {
        setValue(8);
      }
    };

    struct SizeR: Size {
      SizeR()
      {
        setValue(3);
      }
    };

    struct PulseWidthL: PulseWidth {};
    struct PulseWidthR: PulseWidth {};

    using Properties = PropertySet<SizeL, SizeR, PulseWidthL>;
  };


  class Applet : public ArticCircleApplet<Model> {
  public:
    Applet()
    {
      // Maximum 9 characters
      //       123456789
      setName("Tr.2Gate");

      setCallback<Model::SizeL>([this](const auto& value)
      {
          denominator_[0] = value;
          this->updateNumerators();
      });

      setCallback<Model::SizeR>([this](const auto& value)
      {
          denominator_[1] = value;
          this->updateNumerators();
      });

      setCallback<Model::PulseWidthL>([this](const auto& value)
      {
          pulseWidth_[0] = value;
          this->updateNumerators();
      });
/*
      setCallback<Model::PulseWidthR>([this](const auto& value)
      {
          pulseWidth_[1] = value;
          this->updateNumerators();
      */
    }

    virtual void reset() final {
      counter_ = 0;
      state_[0] = state_[1] = false;
      outState_[0] = outState_[1] = false;
    };

    virtual void tick() final
    {
      const auto gate = Gate(0);
      if (gate != lastGate_)
      {
        ForEachChannel(ch)
        {
          const auto cycle = counter_ % denominator_[ch];
          const auto state = cycle < numerator_[ch];
          if (state == gate)
          {
            state_[ch] = state;
          }
        }

        bool output[2];
        output[0] = state_[0] ^ state_[1];
        output[1] = state_[0] ^ !state_[1];

        ForEachChannel(ch)
        {
//          if (gate)
//          {
            if (output[ch]&&!outState_[ch])
            {
              ClockOut(ch);
            }
//          }
          outState_[ch] = output[ch];
        }
        if (gate) counter_++;
        lastGate_ = gate;
      }
    }

    void drawApplet() final
    {
      ArticCircleApplet<NTriggerToGate::Model>::drawApplet();
      gfxSkyline();
      gfxPrint(32,16, numerator_[0]);
      gfxPrint(32,27, numerator_[1]);
      gfxPrint(52,16, state_[0]);
      gfxPrint(52,27, state_[1]);
      gfxPrint(52,38, counter_);
    }

    void updateNumerators()
    {
      const auto calcNumerator= [](const int denominator, const float pulseWidth)
      {
        const auto numerator = int(std::round(denominator * pulseWidth));
        return std::max(std::min(numerator, denominator -1), 1);
      };
      numerator_[0] = calcNumerator(denominator_[0], pulseWidth_[0]);
      const auto pw2 = pulseWidth_[0] > 0.5 ? pulseWidth_[0] * 0.666667f : 1 - pulseWidth_[0];
      numerator_[1] = calcNumerator(denominator_[1], pw2);
    }
  private:
    std::size_t numerator_[2];
    std::size_t denominator_[2];
    float pulseWidth_[2];

    bool state_[2];
    std::size_t counter_;

    bool outState_[2];
    bool lastGate_ = false;
  };

  Applet instance_[2];

} // NTriggerToGate

void TriggerToGate_Start(bool hemisphere) {NTriggerToGate::instance_[hemisphere].BaseStart(hemisphere);}
void TriggerToGate_Controller(bool hemisphere, bool forwarding) {NTriggerToGate::instance_[hemisphere].BaseController(forwarding);}
void TriggerToGate_View(bool hemisphere) {NTriggerToGate::instance_[hemisphere].BaseView();}
void TriggerToGate_OnButtonPress(bool hemisphere) {NTriggerToGate::instance_[hemisphere].OnButtonPress();}
void TriggerToGate_OnEncoderMove(bool hemisphere, int direction) {NTriggerToGate::instance_[hemisphere].OnEncoderMove(direction);}
void TriggerToGate_ToggleHelpScreen(bool hemisphere) {NTriggerToGate::instance_[hemisphere].HelpScreen();}
uint32_t TriggerToGate_OnDataRequest(bool hemisphere) {return NTriggerToGate::instance_[hemisphere].OnDataRequest();}
void TriggerToGate_OnDataReceive(bool hemisphere, uint32_t data) {NTriggerToGate::instance_[hemisphere].OnDataReceive(data);}
