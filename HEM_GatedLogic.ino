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

class ClockedLogic : public HemisphereApplet {
public:

  enum class Operant_t
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

  struct Model
  {
    struct OperantBase : EnumProperty<Operant_t>
    {
      OperantBase()
      {
        setValue(Operant_t::source);

        setEnumStrings({
          {Operant_t::source, "Src"},
          {Operant_t::invert, "Not"},
          {Operant_t::a, "And"},
          {Operant_t::o, "Or"},
          {Operant_t::x, "Xor"}
        });          
      }
    };
    
    struct OperantL : OperantBase
    {
    };

    struct OperantR : OperantBase
    {
    };

    struct Mode : EnumProperty<Mode_t>
    {
      Mode()
      {
        setValue(Mode_t::triggered);
        
        setEnumStrings({
          {Mode_t::triggered, "Trg"},
          {Mode_t::clocked, "Clk"},
        });          
      }
    };

    using Properties = PropertySet<OperantL, OperantR, Mode>;
  };
  
    const char* applet_name() { // Maximum 10 characters
        return "ClockedLogic";
    }

	/* Run when the Applet is selected */
    void Start()
    {
      cursor_ = 0;
      operant_[0] = operant_[1] = Operant_t::source;
      mode_ = { Mode_t::triggered };
    }

	/* Run during the interrupt service routine, 16667 times per second */
    void Controller() {
    }

	/* Draw the screen */
    void View() {
        gfxHeader(applet_name());
        gfxSkyline();

        const char* operantString[int(Operant_t::COUNT)] = { "Src", "Inv", "And" , "Or", "Xor" };
        const char* modeString[int(Mode_t::COUNT)] = { "Trg", "Clk"};

        gfxPrint(1, 15, operantString[int(operant_[0])]);
        gfxPrint(21, 15, modeString[int(mode_)]);
        gfxPrint(41, 15, operantString[int(operant_[1])]);
    }

	/* Called when the encoder button for this hemisphere is pressed */
    void OnButtonPress() {
      propertyManager_.next();
    }

	/* Called when the encoder for this hemisphere is rotated
	 * direction 1 is clockwise
	 * direction -1 is counterclockwise
	 */
    void OnEncoderMove(int direction) {
      propertyManager_.updateCurrent(direction);
    }

    /* Each applet may save up to 32 bits of data. When data is requested from
     * the manager, OnDataRequest() packs it up (see HemisphereApplet::Pack()) and
     * returns it.
     */
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        // example: pack property_name at bit 0, with size of 8 bits
        // Pack(data, PackLocation {0,8}, property_name);
        return data;
    }

    /* When the applet is restored (from power-down state, etc.), the manager may
     * send data to the applet via OnDataReceive(). The applet should take the data
     * and unpack it (see HemisphereApplet::Unpack()) into zero or more of the applet's
     * properties.
     */
    void OnDataReceive(uint32_t data) {
        // example: unpack value at bit 0 with size of 8 bits to property_name
        // property_name = Unpack(data, PackLocation {0,8});
    }

protected:
    /* Set help text. Each help section can have up to 18 characters. Be concise! */
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "Digital in help";
        help[HEMISPHERE_HELP_CVS]      = "CV in help";
        help[HEMISPHERE_HELP_OUTS]     = "Out help";
        help[HEMISPHERE_HELP_ENCODER]  = "123456789012345678";
        //                               "------------------" <-- Size Guide
    }

private:
  PropertyManager<Model> propertyManager_;
  Operant_t operant_[2];
  Mode_t mode_;

  int cursor_;
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to ClockedLogic,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
ClockedLogic ClockedLogic_instance[2];

void ClockedLogic_Start(bool hemisphere) {ClockedLogic_instance[hemisphere].BaseStart(hemisphere);}
void ClockedLogic_Controller(bool hemisphere, bool forwarding) {ClockedLogic_instance[hemisphere].BaseController(forwarding);}
void ClockedLogic_View(bool hemisphere) {ClockedLogic_instance[hemisphere].BaseView();}
void ClockedLogic_OnButtonPress(bool hemisphere) {ClockedLogic_instance[hemisphere].OnButtonPress();}
void ClockedLogic_OnEncoderMove(bool hemisphere, int direction) {ClockedLogic_instance[hemisphere].OnEncoderMove(direction);}
void ClockedLogic_ToggleHelpScreen(bool hemisphere) {ClockedLogic_instance[hemisphere].HelpScreen();}
uint32_t ClockedLogic_OnDataRequest(bool hemisphere) {return ClockedLogic_instance[hemisphere].OnDataRequest();}
void ClockedLogic_OnDataReceive(bool hemisphere, uint32_t data) {ClockedLogic_instance[hemisphere].OnDataReceive(data);}
