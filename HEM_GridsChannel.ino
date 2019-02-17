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
      BD_ALT,
      SD_ALT,
      HH_ALT,
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
        setLabel("Ds");
      }
    };

    struct X: Property<int>
    {
      X()
      {
        setRange(0, 255);
        setValue(128);
      }
    };

    struct Y: Property<int>
    {
      Y()
      {
        setRange(0, 255);
        setValue(128);
      }
    };

    struct Mode: Property<Modes>
    {
      Mode()
      {
        setValue(Modes::BD);
        setEnumStrings({"BD", "SD", "HH", "B-", "S-", "H-"});
      }
    };

    struct ModeL: Mode{};
    struct ModeR: Mode{};

    using Properties = PropertySet<ModeL , ModeR , Density, X, Y>;
  };

  class Applet: public ArticCircleApplet<Model>
  {
  public:
    Applet()
    {
      setName("Grid");

      bind<Model::ModeL>(mode_[0]);
      bind<Model::ModeR>(mode_[1]);

      setCallback<Model::Density>([this](const float &d){
        density_ = int(d * 255);
      });

      bind<Model::X>(x_);
      bind<Model::Y>(y_);
    }

    virtual void reset() final
    {
      channel_.reset();
      output_[0] = output_[1] = false;
    }

    void processStep()
    {
        const auto density =  constrain(density_ + Proportion(DetentedIn(0), HEMISPHERE_MAX_CV, 256), 0, 256);
        const uint8_t threshold = ~density;

        ForEachChannel(ch)
        {
          const auto channel = grids::Channel::Selector(mode_[ch]);
          const auto level = channel_.level(channel, x_, y_);

          if (level > threshold)
          {
            if (density == 255) // Output levels
            {
              Out(ch, Proportion(level, 256, HEMISPHERE_MAX_CV));
            }
            else
            {
              output_[ch] = !output_[ch];
              GateOut(ch, output_[ch]);
//              ClockOut(ch); // trigger
            }
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
    Model::Modes mode_[2];

    bool output_[2];
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
