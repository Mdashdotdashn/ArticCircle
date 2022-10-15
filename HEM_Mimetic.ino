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

namespace NMimetic
{
  template <typename tProperty>
  struct RandomValueGenerator: public detail::IValueConverter
  {
    RandomValueGenerator(tProperty& p)
    : property_(p) {}

    void update(int direction) override
    {
      property_.setValue(random_.seed());
    }
  private:
    tProperty& property_;
    Random<sample_t> random_;
  };

  struct Model
  {
    struct Seed: Property<uint32_t>
    {
      Seed()
      {
      }
      using ValueConverter = RandomValueGenerator<Seed>;
    };
    using Properties = PropertySet<Seed>;
  };


  class Applet : public ArticCircleApplet<Model> {
  public:
    Applet()
    {
      // Maximum 9 characters
      //       123456789
      setName("Mimetic");

      setCallback<Model::Seed>([this](const auto& seed){
        this->seed_ = seed;
      });

      length_ = 16;
      position_ = 0;
      reset_ = true;
    }

    virtual void reset() final
    {
    }

    virtual void tick() final
    {
      reset_ = reset_ || Gate(1);
      if (Clock(0))
      {
        position_ = (position_ + 1) % length_;

        if (reset_)
        {
            position_ = 0;
            Out(1, 1.f * HEMISPHERE_MAX_CV);
        }
        else
        {
            Out(0, 1.f * HEMISPHERE_MAX_CV);
        }
        reset_ = false;
        if (position_ == 0)
        {
          rand_.seed(seed_);
        }
        const auto value = rand_.tick();
        Out(0, float(value) * HEMISPHERE_MAX_CV);
      }
    }

    void drawApplet() final
    {
      ArticCircleApplet<NMimetic::Model>::drawApplet();
      gfxSkyline();
    }

  private:
    uint16_t position_;
    uint16_t length_;
    uint32_t seed_;
    bool reset_;
    Random<sample_t> rand_;
  };

  Applet instance_[2];

} // NMimetic

void Mimetic_Start(bool hemisphere) {NMimetic::instance_[hemisphere].BaseStart(hemisphere);}
void Mimetic_Controller(bool hemisphere, bool forwarding) {NMimetic::instance_[hemisphere].BaseController(forwarding);}
void Mimetic_View(bool hemisphere) {NMimetic::instance_[hemisphere].BaseView();}
void Mimetic_OnButtonPress(bool hemisphere) {NMimetic::instance_[hemisphere].OnButtonPress();}
void Mimetic_OnEncoderMove(bool hemisphere, int direction) {NMimetic::instance_[hemisphere].OnEncoderMove(direction);}
void Mimetic_ToggleHelpScreen(bool hemisphere) {NMimetic::instance_[hemisphere].HelpScreen();}
uint32_t Mimetic_OnDataRequest(bool hemisphere) {return NMimetic::instance_[hemisphere].OnDataRequest();}
void Mimetic_OnDataReceive(bool hemisphere, uint32_t data) {NMimetic::instance_[hemisphere].OnDataReceive(data);}
