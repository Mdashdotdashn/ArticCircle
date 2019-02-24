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
    struct Denominator: Property<int>
    {
      Denominator()
      {
        setRange(2,24);
        setValue(2);        
      }
    };

    struct Numerator: Property<int>
    {
      Numerator()
      {
        setRange(1,23);
        setValue(1);        
      }
    };

    struct DenominatorL: Denominator {};
    struct DenominatorR: Denominator {};

    struct NumeratorL: Numerator {};
    struct NumeratorR: Numerator {};

    using Properties = PropertySet<DenominatorL, NumeratorL, DenominatorR, NumeratorR>;
  };


  class Applet : public ArticCircleApplet<Model> {
  public:
    Applet()
    {
      // Maximum 9 characters
      //       123456789
      setName("Tr.2Gate");

      setCallback<Model::DenominatorL>([this](const auto& value)
      {
          denominator_[0] = value;
          this->setRange<Model::NumeratorL>(1, value -1);
      });

      setCallback<Model::DenominatorR>([this](const auto& value)
      {
          denominator_[1] = value;
          this->setRange<Model::NumeratorR>(1, value -1);
      });
      
      bind<Model::NumeratorL>(numerator_[0]);
      bind<Model::NumeratorR>(numerator_[1]);
    }

    virtual void reset() final {
      count_ = 0;
    };

    virtual void tick() final
    {
      if (Clock(0))
      {
        ForEachChannel(ch)
        {
          const auto cycle = count_ % denominator_[ch];
          const auto high = cycle < numerator_[ch];
          GateOut(ch, high);
        }
        count_++;
      }
    }

    void drawApplet() final
    {
      ArticCircleApplet<NTriggerToGate::Model>::drawApplet();
      gfxSkyline();
    }

  private:
    int numerator_[2];
    int denominator_[2];
    std::size_t count_;
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
