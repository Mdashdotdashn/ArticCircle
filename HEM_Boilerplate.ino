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

namespace NClassName
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
      setName("ClassName");
    }

    virtual std::pair<int,int> tick(const std::pair<bool, bool>& gateIn, const std::pair<int,int>& cvIn) final
    {
      return {0,0};
    }

    void drawApplet() final
    {
      ArticCircleApplet<NClassName::Model>::drawApplet();
    }

  private:
  };

  Applet instance_[2];

} // NClassName

void ClassName_Start(bool hemisphere) {NClassName::instance_[hemisphere].BaseStart(hemisphere);}
void ClassName_Controller(bool hemisphere, bool forwarding) {NClassName::instance_[hemisphere].BaseController(forwarding);}
void ClassName_View(bool hemisphere) {NClassName::instance_[hemisphere].BaseView();}
void ClassName_OnButtonPress(bool hemisphere) {NClassName::instance_[hemisphere].OnButtonPress();}
void ClassName_OnEncoderMove(bool hemisphere, int direction) {NClassName::instance_[hemisphere].OnEncoderMove(direction);}
void ClassName_ToggleHelpScreen(bool hemisphere) {NClassName::instance_[hemisphere].HelpScreen();}
uint32_t ClassName_OnDataRequest(bool hemisphere) {return NClassName::instance_[hemisphere].OnDataRequest();}
void ClassName_OnDataReceive(bool hemisphere, uint32_t data) {NClassName::instance_[hemisphere].OnDataReceive(data);}
