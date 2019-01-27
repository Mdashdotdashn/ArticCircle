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

namespace NClockedLogic
{
  struct Model
  {
    enum class Operator_t
    {
      source,
      invert,
      a,
      o,
      x,
      COUNT
    };
  
    enum class Mode_t
    {
      triggered,
      clocked,
      COUNT
    };
    
    struct OperatorBase : Property<Operator_t>
    {
      OperatorBase()
      {
        setValue(Operator_t::source);

        setEnumStrings({"Src", "Not", "And", "Or", "Xor"});        
      }
    };
    
    struct OperatorL : OperatorBase
    {};

    struct OperatorR : OperatorBase
    {};

    struct Mode : Property<Mode_t>
    {
      Mode()
      {
        setValue(Mode_t::triggered);
        setEnumStrings({ "Trg", "Clk", });
      }
    };

    using Properties = PropertySet<OperatorL, OperatorR, Mode>;
  }; 


  class Applet : public ArticCircleApplet<NClockedLogic::Model> {
  public:
    Applet() 
    { 
      // Maximum 9 characters
      //       123456789
      setName("Clk.Logic");

      bind<Model::Mode>(mode_);
      bind<Model::OperatorL>(operatorL_);
      bind<Model::OperatorR>(operatorR_);
    }

    virtual std::pair<int,int> tick(const std::pair<bool, bool>& gateIn, const std::pair<int,int>& cvIn) final
    {
      if (mode_ == Model::Mode_t::triggered)
      {
        return cvIn;
      }
      return { 0, 0} ;
    }

    void drawApplet() final
    {
      ArticCircleApplet<NClockedLogic::Model>::drawApplet();
    }

    private:
    Model::Mode_t mode_;
    Model::Operator_t operatorL_;
    Model::Operator_t operatorR_;
  };

  Applet instance_[2];

} // NClockedLogic

void ClockedLogic_Start(bool hemisphere) {NClockedLogic::instance_[hemisphere].BaseStart(hemisphere);}
void ClockedLogic_Controller(bool hemisphere, bool forwarding) {NClockedLogic::instance_[hemisphere].BaseController(forwarding);}
void ClockedLogic_View(bool hemisphere) {NClockedLogic::instance_[hemisphere].BaseView();}
void ClockedLogic_OnButtonPress(bool hemisphere) {NClockedLogic::instance_[hemisphere].OnButtonPress();}
void ClockedLogic_OnEncoderMove(bool hemisphere, int direction) {NClockedLogic::instance_[hemisphere].OnEncoderMove(direction);}
void ClockedLogic_ToggleHelpScreen(bool hemisphere) {NClockedLogic::instance_[hemisphere].HelpScreen();}
uint32_t ClockedLogic_OnDataRequest(bool hemisphere) {return NClockedLogic::instance_[hemisphere].OnDataRequest();}
void ClockedLogic_OnDataReceive(bool hemisphere, uint32_t data) {NClockedLogic::instance_[hemisphere].OnDataReceive(data);}

