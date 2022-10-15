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

namespace NNoiseRampLfo
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
      setName("NzRmpLfo");
      mPhasor.reset(kSampleRate);
      mPhasor.setFrequency(mLfoFrequency);
    }

    virtual void reset() final {};

    virtual void tick() final
    {
      mPhasor.tick();
      if (mPhasor.flanked())
      {
        // new target between zero and one
        const auto target = rand_.tick() * sample_t(2) - sample_t(1);
        const auto offset = target - mValue;
        mInc = sample_t(mLfoFrequency / kSampleRate) * offset;
      }
      mValue += mInc;
      Out(0, float(mValue) * HEMISPHERE_3V_CV);
    }

    void drawApplet() final
    {
      ArticCircleApplet<NNoiseRampLfo::Model>::drawApplet();
    }

  private:
    Phasor<sample_t> mPhasor;
    float mLfoFrequency = .5f;
    sample_t mInc = sample_t(0.001);
    sample_t mValue = sample_t(0);
    Random<sample_t> rand_;
  };

   Applet instance_[2];
} // NNoiseRampLfo

void NoiseRampLfo_Start(bool hemisphere) {NNoiseRampLfo::instance_[hemisphere].BaseStart(hemisphere);}
void NoiseRampLfo_Controller(bool hemisphere, bool forwarding) {NNoiseRampLfo::instance_[hemisphere].BaseController(forwarding);}
void NoiseRampLfo_View(bool hemisphere) {NNoiseRampLfo::instance_[hemisphere].BaseView();}
void NoiseRampLfo_OnButtonPress(bool hemisphere) {NNoiseRampLfo::instance_[hemisphere].OnButtonPress();}
void NoiseRampLfo_OnEncoderMove(bool hemisphere, int direction) {NNoiseRampLfo::instance_[hemisphere].OnEncoderMove(direction);}
void NoiseRampLfo_ToggleHelpScreen(bool hemisphere) {NNoiseRampLfo::instance_[hemisphere].HelpScreen();}
uint32_t NoiseRampLfo_OnDataRequest(bool hemisphere) {return NNoiseRampLfo::instance_[hemisphere].OnDataRequest();}
void NoiseRampLfo_OnDataReceive(bool hemisphere, uint32_t data) {NNoiseRampLfo::instance_[hemisphere].OnDataReceive(data);}
