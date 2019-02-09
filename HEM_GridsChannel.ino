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

#include "grids.h"


namespace NGridsChannel
{
  struct Model
  {
    enum class Modes
    {
      BD,
      SD,
      HH,
      BS,
      BH,
      SH,
      COUNT
    };

    struct PercentageProperty : public Property<float>
    {
      PercentageProperty()
      {
        setRange(0.f, 1.f);
      }
    };

    struct Density: PercentageProperty
    {
      Density()
      {
        setValue(0.5f);
      }
    };

    struct X: PercentageProperty
    {
      X()
      {
        setValue(0.5f);
      }
    };

    struct Y: PercentageProperty
    {
      Y()
      {
        setValue(0.5f);
      }
    };

    struct Mode: Property<Modes>
    {
      Mode()
      {
        setValue(Modes::BD);
        setEnumStrings({"BD", "SD", "HH", "BS", "BH", "SH"});
      }
    };

    using Properties = PropertySet<Mode, Density, X, Y>;
  };

  class Applet: public ArticCircleApplet<Model>
  {
  public:
    Applet()
    {
      setName("Grid");

      bind<Model::Mode>(mode_);
      setCallback<Model::Density>([this](const float &d){
        density_ = int(d * 255);
      });
      setCallback<Model::X>([this](const float &x){
        x_ = int(x * 255);
      });
      setCallback<Model::Y>([this](const float &y){
        y_ = int(y * 255);
      });
    }

    virtual void reset() final
    {
      channel_.reset();
    }

    void processStep()
    {
        const auto density =  constrain(density_ + Proportion(DetentedIn(0), HEMISPHERE_MAX_CV, 256), 0, 256);
        const uint8_t threshold = ~density;

        const auto isSingleOutput = [](Model::Modes mode)
        {
          return (mode == Model::Modes::BD || mode == Model::Modes::SD || mode == Model::Modes::HH);
        };
 
        if (isSingleOutput(mode_))
        {
          const auto channel = uint8_t(mode_);
          const auto level = channel_.level(channel, x_, y_);
          if (level > threshold)
          {
            ClockOut(0); // trigger
            Out(1, Proportion(level - threshold, 256 - threshold, HEMISPHERE_MAX_CV));
          }
        }
        else // dual
        {
          uint8_t left = (mode_ == Model::Modes::BS) || (mode_ == Model::Modes::BH) ? 0 : 1;
          uint8_t right = (mode_ == Model::Modes::BH) || (mode_ == Model::Modes::SH) ? 2 : 1;

          uint8_t levels[2] =
            {
              channel_.level(left, x_, y_),
              channel_.level(right, x_, y_)
            };

          if (density == 255) // Output levels
          {
            Out(0, Proportion(levels[0], 256, HEMISPHERE_MAX_CV));
            Out(1, Proportion(levels[1], 256, HEMISPHERE_MAX_CV));
          }
          else // Output trigger
          {
            if (levels[0] > threshold) ClockOut(0);
            if (levels[1] > threshold) ClockOut(1);
          }
        }
    }

    void tick() final
    {
      if (Clock(1))
      {
        channel_.reset();
      }

      if (Clock(0))
      {
        processStep();
        channel_.advance();
      }
    }

    /* Draw the screen */
    void drawApplet() final
    {
      ArticCircleApplet<Model>::drawApplet();

      gfxSkyline();

      gfxPrint(31,25, density_);
    }

  private:
    uint8_t x_;
    uint8_t y_;
    uint8_t density_;
    Model::Modes mode_;

    grids::Channel channel_;
  };

  Applet instance_[2];
}

void GridsChannel_Start(bool hemisphere) {NGridsChannel::instance_[hemisphere].BaseStart(hemisphere);}
void GridsChannel_Controller(bool hemisphere, bool forwarding) {NGridsChannel::instance_[hemisphere].BaseController(forwarding);}
void GridsChannel_View(bool hemisphere) {NGridsChannel::instance_[hemisphere].BaseView();}
void GridsChannel_OnButtonPress(bool hemisphere) {NGridsChannel::instance_[hemisphere].OnButtonPress();}
void GridsChannel_OnEncoderMove(bool hemisphere, int direction) {NGridsChannel::instance_[hemisphere].OnEncoderMove(direction);}
void GridsChannel_ToggleHelpScreen(bool hemisphere) {NGridsChannel::instance_[hemisphere].HelpScreen();}
uint32_t GridsChannel_OnDataRequest(bool hemisphere) {return NGridsChannel::instance_[hemisphere].OnDataRequest();}
void GridsChannel_OnDataReceive(bool hemisphere, uint32_t data) {NGridsChannel::instance_[hemisphere].OnDataReceive(data);}
