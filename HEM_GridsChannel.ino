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
      BD_HALF,
      SD_HALF,
      HH_HALF,
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
      BD_HALF,
      SD_HALF,
      HH_HALF,
      COUNT
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
        setEnumStrings({"BD", "SD", "HH", "B-", "S-", "H-", "B/", "S/", "H/"});
      }
    };

    struct ModeR: Property<ModesR>
    {
      ModeR()
      {
        setValue(ModesR::PERCENTAGE);
        setEnumStrings({"% ","BD", "SD", "HH","B-", "S-", "H-", "B/", "S/", "H/"});
      }
    };


    using Properties = PropertySet<ModeL , DensityL, ModeR, DensityR, X, Y>;
  };

  namespace detail
  {
    // This one uses standard grids value
    uint8_t calcStandardLevel(grids::Channel& channel, const grids::Channel::Selector selector, const uint8_t x ,const uint8_t y)
    {
      return channel.level(selector, x, y);
    }

    // this one returns the difference between two points located at different place on the map
    uint8_t calcAlternateLevel(grids::Channel& channel, const grids::Channel::Selector selector, const uint8_t x ,const uint8_t y)
    {
      const auto level1 = channel.level(selector, x, y);

      const auto altX = (x + 128) % 256;
      const auto altY =  (y + 128) % 256;
      const auto level2 = channel.level(selector, altX, altY);
      return (level2 > level1) ? (level2 - level1) : (level1 - level2);
    }
  }

  class Applet: public ArticCircleApplet<Model>
  {
  public:
    Applet()
    {
      setName("Grids");

      setCallback<Model::ModeL>([this](const auto& m) {
        this->updateLeftMode(m);
      });

      setCallback<Model::ModeR>([this](const auto& m) {
        this->updateRightMode(m);
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

      setPosition<Model::X>(12,20);
      setPosition<Model::Y>(12,29);
    }

    virtual void reset() final
    {
      channel_.reset();
      flopState_[0] = flopState_[1] = false;
    }

    void processStep()
    {
        ForEachChannel(ch)
        {
          if ((ch == 0) || (!percentageOnRight_))
          {
            const auto density =  constrain(density_[ch] + Proportion(DetentedIn(0), HEMISPHERE_MAX_CV, 256), 0, 256);
            const uint8_t threshold = ~density;

            const auto level =  (processor_[ch]) ? processor_[ch](channel_, selector_[ch], x_, y_) : 0;

            if (density_[ch] == 255) // Output levels
            {
              Out(ch, Proportion(level, 256, HEMISPHERE_MAX_CV));
              if (percentageOnRight_)
              {
                Out(1, Proportion(level, 256, HEMISPHERE_MAX_CV));
              }
            }
            else
            {
              bool trigger = level > threshold;
              if (trigger && flipFlopOutput_[ch])
              {
                flopState_[ch] = !flopState_[ch];
                trigger = flopState_[ch];
              }

              sizer_[ch].feed(trigger);

              if (trigger)
              {
                ClockOut(ch); // trigger
                // If we're using the percentage mode, sent it to output 1
                if (percentageOnRight_)
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
      ForEachChannel(ch)
      {
        const auto size = sizer_[ch].updateSize();
        if (size > 0)
        {
          const auto offset = 32 * ch;
          gfxRect(offset + 2, 38, 20, 20);
        }
      }
    }

    void updateLeftMode(Model::ModesL m)
    {
      int channel = int(m) % 3;
      selector_[0] = grids::Channel::Selector(channel);

      switch(m)
      {
        case Model::ModesL::BD_ALT:
        case Model::ModesL::SD_ALT:
        case Model::ModesL::HH_ALT:
          processor_[0] =  detail::calcAlternateLevel;
          break;
        default:
          processor_[0] =  detail::calcStandardLevel;
      }
      flipFlopOutput_[0] = m > Model::ModesL::HH_ALT;
    }

    void updateRightMode(Model::ModesR m)
    {
      int channel = (int(m) - 1) % 3;
      selector_[1] = grids::Channel::Selector(channel);

      switch(m)
      {
        case Model::ModesR::BD_ALT:
        case Model::ModesR::SD_ALT:
        case Model::ModesR::HH_ALT:
          processor_[1] =  detail::calcAlternateLevel;
          break;
        default:
          processor_[1] =  detail::calcStandardLevel;
      }

      percentageOnRight_ = (m == Model::ModesR::PERCENTAGE);
      flipFlopOutput_[1] = m > Model::ModesR::HH_ALT;

      this->setVisibility<Model::DensityR>(m != Model::ModesR::PERCENTAGE);
    }

  private:
    uint8_t x_;
    uint8_t y_;
    uint8_t density_[2];
    grids::Channel::Selector selector_[2];

    using ProcessorFn = std::function<uint8_t(grids::Channel&, const grids::Channel::Selector, const uint8_t,const uint8_t)>;
    ProcessorFn processor_[2];

    bool percentageOnRight_;
    bool flipFlopOutput_[2];
    bool flopState_[2];
    grids::Channel channel_;

    TriggerSizer<16, 24> sizer_[2];
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
