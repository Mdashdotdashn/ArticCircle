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

namespace NPingableSyncopation
{
  struct Model
  {
    struct Denominator: Property<int>
    {
      Denominator()
      {
        setValue(4);
        setRange(1, 12);
      }
    };
    using Properties = PropertySet<Denominator>;
  };

  class Applet : public ArticCircleApplet<Model> {
  public:
    Applet()
    {
      // Maximum 9 characters
      //       123456789
      setName("Ping Sync");

      setCallback<Model::Denominator>([this](const int v){
        this->divider_.setAmount(v);
      });
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
      const auto value = divider_.tick(phaser_.tick());

      lastGate_ =gate;
      return {float(value) * HEMISPHERE_MAX_CV,0};
    }

    void drawApplet() final
    {
      ArticCircleApplet<NPingableSyncopation::Model>::drawApplet();
      gfxSkyline();
    }

  private:
    PingablePhaser phaser_;
    PhaserDivider divider_;
    bool lastGate_ = false;
  };

  Applet instance_[2];

} // NPingableSyncopation

void PingableSyncopation_Start(bool hemisphere) {NPingableSyncopation::instance_[hemisphere].BaseStart(hemisphere);}
void PingableSyncopation_Controller(bool hemisphere, bool forwarding) {NPingableSyncopation::instance_[hemisphere].BaseController(forwarding);}
void PingableSyncopation_View(bool hemisphere) {NPingableSyncopation::instance_[hemisphere].BaseView();}
void PingableSyncopation_OnButtonPress(bool hemisphere) {NPingableSyncopation::instance_[hemisphere].OnButtonPress();}
void PingableSyncopation_OnEncoderMove(bool hemisphere, int direction) {NPingableSyncopation::instance_[hemisphere].OnEncoderMove(direction);}
void PingableSyncopation_ToggleHelpScreen(bool hemisphere) {NPingableSyncopation::instance_[hemisphere].HelpScreen();}
uint32_t PingableSyncopation_OnDataRequest(bool hemisphere) {return NPingableSyncopation::instance_[hemisphere].OnDataRequest();}
void PingableSyncopation_OnDataReceive(bool hemisphere, uint32_t data) {NPingableSyncopation::instance_[hemisphere].OnDataReceive(data);}
