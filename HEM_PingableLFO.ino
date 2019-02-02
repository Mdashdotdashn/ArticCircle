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

namespace NPingableLfo
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
      setName("Ping LFO");
    }

    virtual void reset() final
    {
      phaser_.reset(kSampleRate);
    }

    virtual std::pair<int,int> tick(const std::pair<bool, bool>& gateIn, const std::pair<int,int>& cvIn) final
    {
      const bool gate = Gate(0);
      if (gate && (lastGate_ != gate))
      {
        phaser_.ping(OC::CORE::ticks);
      }
      lastGate_ =gate;
      return {float(phaser_.tick()) * HEMISPHERE_MAX_CV,0};
    }

    void drawApplet() final
    {
      ArticCircleApplet<NPingableLfo::Model>::drawApplet();

      gfxPrintF(25,15,phaser_.tempo());
      gfxSkyline();
    }

  private:
    PingablePhaser phaser_;
    bool lastGate_ = false;
  };

  Applet instance_[2];

} // NPingableLfo

void PingableLfo_Start(bool hemisphere) {NPingableLfo::instance_[hemisphere].BaseStart(hemisphere);}
void PingableLfo_Controller(bool hemisphere, bool forwarding) {NPingableLfo::instance_[hemisphere].BaseController(forwarding);}
void PingableLfo_View(bool hemisphere) {NPingableLfo::instance_[hemisphere].BaseView();}
void PingableLfo_OnButtonPress(bool hemisphere) {NPingableLfo::instance_[hemisphere].OnButtonPress();}
void PingableLfo_OnEncoderMove(bool hemisphere, int direction) {NPingableLfo::instance_[hemisphere].OnEncoderMove(direction);}
void PingableLfo_ToggleHelpScreen(bool hemisphere) {NPingableLfo::instance_[hemisphere].HelpScreen();}
uint32_t PingableLfo_OnDataRequest(bool hemisphere) {return NPingableLfo::instance_[hemisphere].OnDataRequest();}
void PingableLfo_OnDataReceive(bool hemisphere, uint32_t data) {NPingableLfo::instance_[hemisphere].OnDataReceive(data);}
