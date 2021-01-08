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
    enum class Modes
    {
      Gate,
      Drums,
      COUNT
    };

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

    struct Mode: Property<Modes>
    {
      Mode()
      {
        setValue(Modes::Drums);
        setEnumStrings({"Gate", "Drum"});
      }
    };

    using Properties = PropertySet<StepCount, Density, Mode>;
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

      setCallback<Model::Mode>([this](const auto& m) {
        mode_ = m;
        this->rebuild();
      });
    }

    void rebuild()
    {
      const auto steps = generator_.generate<kMaxSteps>(stepCount_, density_);

      switch(mode_)
      {
        case Model::Modes::Gate:
        {
          bool value = true;
          size_t position = 0;
          for (auto& step : steps)
          {
            for (size_t j = 0; j < step; j++)
            {
              if (position < kMaxSteps) boolSteps_[position++] = value;
            }
            value = ! value;
          }
          break;
        }
        case Model::Modes::Drums:
        {
          for (size_t j = 0; j < kMaxSteps; j++)
          {
            intSteps_[j] = 0;
          }

          uint8_t value = 1u;
          size_t position = 0;
          for (auto& step : steps)
          {
            intSteps_[position] = value;
            position += step;
            value = (value) % 2 + 1;
          }

          // Adds special case for up bead
          const auto drumBassCount = 2;
          const auto offset = kMaxSteps / drumBassCount;
          for (size_t i = 0; i < drumBassCount; i++)
          {
            const auto position = i * offset;
            intSteps_[position] = (intSteps_[position] == 0) ? 1 : size_t(random() * 2 + 1);
          }

          intSteps_[0] = 1;

          for (size_t j = 0; j < kMaxSteps - 1; j++)
          {
            if (intSteps_[j+1] == intSteps_[j])
            {
              intSteps_[j+1] = 0;
            }
          }
          break;
        }
        default:
          break;
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
        switch(mode_)
        {
          case Model::Modes::Gate:
          {
            // Trigger at output 1 at each beginning of the cycle
            if (position_ == 0) ClockOut(1);
            // Sends the flip-flip state on output 0
            GateOut(0, boolSteps_[position_]);
            break;
          }
          case Model::Modes::Drums:
          {
            switch(intSteps_[position_])
            {
              case 1:
                ClockOut(0);
                break;
              case 2:
                ClockOut(1);
                break;
              default:
                break;
            }
          }
          default:
            break;
        }
        position_ = (position_ +1) % kMaxSteps;
      }
    }

    void drawApplet() final
    {
      ArticCircleApplet<NFlipFlopPattern::Model>::drawApplet();
      gfxSkyline();
    }

    uint32_t OnDataRequest() override {
      using namespace OC;
      // roll current pattern into save data
      Pattern&p =  user_patterns[hemisphere ? 0 : 1];
      switch(mode_)
      {
        case Model::Modes::Gate:
          for (int i = 0; i < 16; i++)
          {
            p.notes[i] = boolSteps_[i] ? 36: 0;
          }
          break;
        case Model::Modes::Drums:
          for (int i = 0; i < 16; i++)
          {
            p.notes[i] = (intSteps_[i] == 0) ? 0 : intSteps_[i] + 35; // Kick - Sidestick
          }
          break;
        default:
          break;
      }

      // store mode
      uint32_t data = 0;
      Pack(data, PackLocation {0,8}, int(mode_));
      return data;
    }

    void OnDataReceive(uint32_t data) override {
      using namespace OC;

      const auto mode = Model::Modes(Unpack(data, PackLocation {0,8}));
      setValue<Model::Mode>(mode);

      // Unroll pattern from data
      Pattern&p =  user_patterns[hemisphere ? 0 : 1];
      switch(mode_)
      {
        case Model::Modes::Gate:
          for (int i = 0; i < 16; i++)
          {
            boolSteps_[i] = p.notes[i] != 0;
          }
          break;
        case Model::Modes::Drums:
          for (int i = 0; i < 16; i++)
          {
            intSteps_[i] = (p.notes[i] == 0) ? 0 : p.notes[i] - 35;
          }
          break;
        default:
          break;
      }
    }
  private:
    FlipFlopPatternWeaver<kMaxSteps> generator_;
    std::array<bool, kMaxSteps> boolSteps_;
    std::array<uint8_t, kMaxSteps> intSteps_;
    float density_;
    size_t stepCount_;
    size_t position_ = 0;
    Model::Modes mode_ = Model::Modes::Drums;
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
