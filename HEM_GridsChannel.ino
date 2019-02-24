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
    enum class ModesL
    {
      BD,
      SD,
      HH,
      BD_ALT,
      SD_ALT,
      HH_ALT,
      COUNT
    };

    enum class ModesR
    {
      PERCENTAGE,
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
      }
    };

    struct DensityL: Density {};
    struct DensityR: Density {};

    struct X: Property<int>
      {
      X()
      {
        setRange(0, 255);
        setValue(128);
        setLabel("X");
      }
    };

    struct Y: Property<int>
    {
      Y()
      {
        setRange(0, 255);
        setValue(128);
        setLabel("Y");
      }
    };

    struct ModeL: Property<ModesL>
    {
      ModeL()
      {
        setValue(ModesL::BD);
        setEnumStrings({"BD", "SD", "HH", "B-", "S-", "H-"});
      }
    };

    struct ModeR: Property<ModesR>
    {
      ModeR()
      {
        setValue(ModesR::PERCENTAGE);
        setEnumStrings({"% ","BD", "SD", "HH", "B-", "S-", "H-"});
      }
    };


    using Properties = PropertySet<ModeL , DensityL, ModeR, DensityR, X, Y>;
  };

  class Applet: public ArticCircleApplet<Model>
  {
  public:
    Applet()
    {
      setName("Grids");

      bind<Model::ModeL>(modeL_);
      
      setCallback<Model::ModeR>([this](const auto& m) {
        modeR_ = m;
        this->setVisibility<Model::DensityR>(m != Model::ModesR::PERCENTAGE);
      });

      setCallback<Model::DensityL>([this](const float &d){
        density_[0] = int(d * 255);
      });

      setCallback<Model::DensityR>([this](const float &d){
        density_[1] = int(d * 255);
      });

      bind<Model::X>(x_);
      bind<Model::Y>(y_);

      setPosition<Model::ModeL>(0,0);
      setPosition<Model::DensityL>(0,9);
      setPosition<Model::ModeR>(29,0);
      setPosition<Model::DensityR>(29,9);
    }

    virtual void reset() final
    {
      channel_.reset();
    }

    void processStep()
    {
        ForEachChannel(ch)
        {
          if (((ch == 0) || (modeR_ != Model::ModesR::PERCENTAGE)))
          {
            const auto density =  constrain(density_[ch] + Proportion(DetentedIn(0), HEMISPHERE_MAX_CV, 256), 0, 256);
            const uint8_t threshold = ~density;

            const auto channel = (ch == 0) ? grids::Channel::Selector(modeL_) :  grids::Channel::Selector(int(modeR_) - 1);  
            const auto level = channel_.level(channel, x_, y_);            

            if (density_[ch] == 255) // Output levels
            {
              Out(ch, Proportion(level, 256, HEMISPHERE_MAX_CV));
              if (modeR_ == Model::ModesR::PERCENTAGE)
              {
                Out(1, Proportion(level, 256, HEMISPHERE_MAX_CV));              
              }
            }
            else
            {
              if (level > threshold)
              {
                ClockOut(ch); // trigger
                // If we're using the percentage mode, sent it to output 1
                if (modeR_ == Model::ModesR::PERCENTAGE)
                {
                    Out(1, Proportion(level - threshold, 256 - threshold, HEMISPHERE_MAX_CV));
                }
              }
              else
              {
                Out(ch, 0);
              }
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
    }

  private:
    uint8_t x_;
    uint8_t y_;
    uint8_t density_[2];
    Model::ModesL modeL_;
    Model::ModesR modeR_;

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
