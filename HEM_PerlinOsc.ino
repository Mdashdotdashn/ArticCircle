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
#include "src/nostromo/perlin.h"

namespace NPerlinOsc
{
  struct Model
  {
    using Properties = PropertySet<>;
  };


  class Applet : public ArticCircleApplet<Model> {
  public:
    Applet()
    {
      // Maximum 9 characters
      //       123456789
      setName("PerlinOsc");
    }

    virtual void reset() final
    {
      oscillator_.setParameters(0.25,0.,0.,0.);
      oscillator_.setFadeCycles(1);

      phaseIncrease_ = sample_t(100.f / kSampleRate);
      phase_ = sample_t(0);
    }

    virtual void tick() final
    {
      phase_ = sample_t::frac(phase_ + phaseIncrease_);
      double value = oscillator_.tick(phase_);
      Out(0, float(value) * HEMISPHERE_3V_CV);
    }

    void drawApplet() final
    {
      ArticCircleApplet<NPerlinOsc::Model>::drawApplet();
    }

  private:
    perlin::Oscillator<sample_t> oscillator_;
    sample_t phaseIncrease_;
    sample_t phase_;
  };

  Applet instance_[2];

} // NPerlinOsc

void PerlinOsc_Start(bool hemisphere) {NPerlinOsc::instance_[hemisphere].BaseStart(hemisphere);}
void PerlinOsc_Controller(bool hemisphere, bool forwarding) {NPerlinOsc::instance_[hemisphere].BaseController(forwarding);}
void PerlinOsc_View(bool hemisphere) {NPerlinOsc::instance_[hemisphere].BaseView();}
void PerlinOsc_OnButtonPress(bool hemisphere) {NPerlinOsc::instance_[hemisphere].OnButtonPress();}
void PerlinOsc_OnEncoderMove(bool hemisphere, int direction) {NPerlinOsc::instance_[hemisphere].OnEncoderMove(direction);}
void PerlinOsc_ToggleHelpScreen(bool hemisphere) {NPerlinOsc::instance_[hemisphere].HelpScreen();}
uint32_t PerlinOsc_OnDataRequest(bool hemisphere) {return NPerlinOsc::instance_[hemisphere].OnDataRequest();}
void PerlinOsc_OnDataReceive(bool hemisphere, uint32_t data) {NPerlinOsc::instance_[hemisphere].OnDataReceive(data);}
