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

namespace NSubHarm
{
  struct Model
  {
    struct Offset : Property<int>
    {
      Offset()
      {
        setLabel("sh");
        setRange(1,10);
        setValue(2);
      }
    };

    using Properties = PropertySet<Offset>;
  };


  class Applet : public ArticCircleApplet<Model> {
  public:
    Applet()
    {
      // Maximum 9 characters
      //       123456789
      setName("SubHarm");

      setCallback<Model::Offset>([this](const auto& o) {
        offset_ = std::log2(1.f/float(o));
      });
    }

    virtual void reset() final {};

    virtual void tick() final
    {
      float cvInput = In(0);
      Out(0, cvInput + offset_ * HEMISPHERE_3V_CV / 3.f);
      Out(1, cvInput);
    }

    void drawApplet() final
    {
      ArticCircleApplet<NSubHarm::Model>::drawApplet();
    }

  private:
    float offset_;
  };

  Applet instance_[2];

} // NSubHarm

void SubHarm_Start(bool hemisphere) {NSubHarm::instance_[hemisphere].BaseStart(hemisphere);}
void SubHarm_Controller(bool hemisphere, bool forwarding) {NSubHarm::instance_[hemisphere].BaseController(forwarding);}
void SubHarm_View(bool hemisphere) {NSubHarm::instance_[hemisphere].BaseView();}
void SubHarm_OnButtonPress(bool hemisphere) {NSubHarm::instance_[hemisphere].OnButtonPress();}
void SubHarm_OnEncoderMove(bool hemisphere, int direction) {NSubHarm::instance_[hemisphere].OnEncoderMove(direction);}
void SubHarm_ToggleHelpScreen(bool hemisphere) {NSubHarm::instance_[hemisphere].HelpScreen();}
uint32_t SubHarm_OnDataRequest(bool hemisphere) {return NSubHarm::instance_[hemisphere].OnDataRequest();}
void SubHarm_OnDataReceive(bool hemisphere, uint32_t data) {NSubHarm::instance_[hemisphere].OnDataReceive(data);}
