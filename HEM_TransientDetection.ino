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

namespace NTransientDetector
{
  struct Model
  {
    struct Release : public Property<float>
    {
      Release()
      {
        setValue(1.f);
        setRange(0.3f, 5.f);
      }
    };

    struct Gain: public Property<float>
    {
      Gain()
      {
        setValue(2.f);
        setRange(0.5f, 5.f);
      };
    };
    using Properties = PropertySet<Release, Gain>;
  };

  class Applet : public ArticCircleApplet<Model>
  {
  public:
    Applet()
    {
      setName("Transient");

      state_ = 0;

      setCallback<Model::Release>([this](const float& v){
        this->releaseCoeff_ = onePoleCoeff(kSampleRate, v);
      });
      setCallback<Model::Gain>([this](const float& v){
        this->gain_ = v;
      });
    }

    virtual std::pair<int,int> tick(const std::pair<bool, bool>& gateIn, const std::pair<int,int>& cvIn) final
    {
        const auto input = sample_t::fromRatio(In(0), HEMISPHERE_3V_CV);
        const auto rectified = abs(input) * gain_;
        state_ = rectified > state_ ? rectified : state_ + (rectified - state_) * releaseCoeff_;
        return {float(state_) * HEMISPHERE_3V_CV, 0};
      }

  	/* Draw the screen */
    void drawApplet() final
    {
      ArticCircleApplet<Model>::drawApplet();

      gfxSkyline();
    }

  private:
    sample_t gain_;
    sample_t state_;
    sample_t releaseCoeff_;
  };

  Applet instance_[2];
}

void TransientDetector_Start(bool hemisphere) {NTransientDetector::instance_[hemisphere].BaseStart(hemisphere);}
void TransientDetector_Controller(bool hemisphere, bool forwarding) {NTransientDetector::instance_[hemisphere].BaseController(forwarding);}
void TransientDetector_View(bool hemisphere) {NTransientDetector::instance_[hemisphere].BaseView();}
void TransientDetector_OnButtonPress(bool hemisphere) {NTransientDetector::instance_[hemisphere].OnButtonPress();}
void TransientDetector_OnEncoderMove(bool hemisphere, int direction) {NTransientDetector::instance_[hemisphere].OnEncoderMove(direction);}
void TransientDetector_ToggleHelpScreen(bool hemisphere) {NTransientDetector::instance_[hemisphere].HelpScreen();}
uint32_t TransientDetector_OnDataRequest(bool hemisphere) {return NTransientDetector::instance_[hemisphere].OnDataRequest();}
void TransientDetector_OnDataReceive(bool hemisphere, uint32_t data) {NTransientDetector::instance_[hemisphere].OnDataReceive(data);}
