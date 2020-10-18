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

namespace NFlipFlopPattern
{
  constexpr auto kMaxSteps = 16u;

  struct Model
  {
    struct StepCount: Property<int>
    {
      StepCount()
      {
        setRange(0, kMaxSteps / 2 - 1);
        setValue(3);
        setLabel("Stp");
      }
    };

    struct Density: PercentageProperty
    {
      Density()
      {
        setValue(0.8f);
        setLabel("D");
      }
    };

    using Properties = PropertySet<StepCount, Density>;
  };


  class Applet : public ArticCircleApplet<Model> {
  public:
    Applet()
    {
      // Maximum 9 characters
      //       123456789
      setName("FlpFlpPtr");

      setCallback<Model::StepCount>([this](const auto& n) {
        stepCount_ = n;
        this->rebuild();
      });

      setCallback<Model::Density>([this](const auto& d) {
        density_ = d;
        this->rebuild();
      });
    }

    void rebuild()
    {
      const auto steps = generator_.generate<kMaxSteps>(stepCount_, density_);
      bool value = true;
      size_t position = 0;
      for (auto& step : steps)
      {
        for (size_t j = 0; j < step; j++)
        {
          if (position < kMaxSteps) steps_[position++] = value;
        }
        value = ! value;
      }
    }

    virtual void reset() final
    {
      rebuild();
    };

    virtual void tick() final
    {
      if (flankUp(1))
      {
          position_ = 0;
      }
    
      if (Clock(0))
      {
        // Trigger at output 1 at each beginning of the cycle
        if (position_ == 0) ClockOut(1);
        // Sends the flip-flip state on output 0
        GateOut(0, steps_[position_]);
        position_ = (position_ +1) % kMaxSteps;
      }
    }

    void drawApplet() final
    {
      ArticCircleApplet<NFlipFlopPattern::Model>::drawApplet();
      gfxSkyline();
    }

  private:
    FlipFlopPatternWeaver<kMaxSteps> generator_;
    std::array<bool, kMaxSteps> steps_;
    float density_;
    size_t stepCount_;
    size_t position_ = 0;
  };

  Applet instance_[2];

} // NFlipFlopPattern

void FlipFlopPattern_Start(bool hemisphere) {NFlipFlopPattern::instance_[hemisphere].BaseStart(hemisphere);}
void FlipFlopPattern_Controller(bool hemisphere, bool forwarding) {NFlipFlopPattern::instance_[hemisphere].BaseController(forwarding);}
void FlipFlopPattern_View(bool hemisphere) {NFlipFlopPattern::instance_[hemisphere].BaseView();}
void FlipFlopPattern_OnButtonPress(bool hemisphere) {NFlipFlopPattern::instance_[hemisphere].OnButtonPress();}
void FlipFlopPattern_OnEncoderMove(bool hemisphere, int direction) {NFlipFlopPattern::instance_[hemisphere].OnEncoderMove(direction);}
void FlipFlopPattern_ToggleHelpScreen(bool hemisphere) {NFlipFlopPattern::instance_[hemisphere].HelpScreen();}
uint32_t FlipFlopPattern_OnDataRequest(bool hemisphere) {return NFlipFlopPattern::instance_[hemisphere].OnDataRequest();}
void FlipFlopPattern_OnDataReceive(bool hemisphere, uint32_t data) {NFlipFlopPattern::instance_[hemisphere].OnDataReceive(data);}
