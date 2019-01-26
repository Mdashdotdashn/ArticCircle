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

class TriggerToGate : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "Tr.->Gt.";
    }

	/* Run when the Applet is selected */
    void Start() {
      gate_ = false;
      countdown_ = 0;
      holdTime_ = 1;
    }

	/* Run during the interrupt service routine, 16667 times per second */
    void Controller() {
      if (Clock(0) && !gate_)
      {
        gate_ = true;
        countdown_ = holdTime_ * 167;
      }

      if (countdown_ > 0)
      {
        countdown_--;
        if (countdown_ ==0)
        {
          gate_ = false;
        }
      }
      GateOut(0, gate_);
      GateOut(1, !gate_);
    }

	/* Draw the screen */
    void View() {
        gfxHeader(applet_name());
        gfxSkyline();
        // Add other view code as private methods
        gfxPrint(21 + pad(100, holdTime_), 15, holdTime_);
    }

	/* Called when the encoder button for this hemisphere is pressed */
    void OnButtonPress() {
    }

	/* Called when the encoder for this hemisphere is rotated
	 * direction 1 is clockwise
	 * direction -1 is counterclockwise
	 */
    void OnEncoderMove(int direction) {
      holdTime_ = clamp(holdTime_ + direction, 1, 100);
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
  bool gate_;
  uint32_t countdown_;

  int holdTime_;
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to TriggerToGate,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
TriggerToGate TriggerToGate_instance[2];

void TriggerToGate_Start(bool hemisphere) {TriggerToGate_instance[hemisphere].BaseStart(hemisphere);}
void TriggerToGate_Controller(bool hemisphere, bool forwarding) {TriggerToGate_instance[hemisphere].BaseController(forwarding);}
void TriggerToGate_View(bool hemisphere) {TriggerToGate_instance[hemisphere].BaseView();}
void TriggerToGate_OnButtonPress(bool hemisphere) {TriggerToGate_instance[hemisphere].OnButtonPress();}
void TriggerToGate_OnEncoderMove(bool hemisphere, int direction) {TriggerToGate_instance[hemisphere].OnEncoderMove(direction);}
void TriggerToGate_ToggleHelpScreen(bool hemisphere) {TriggerToGate_instance[hemisphere].HelpScreen();}
uint32_t TriggerToGate_OnDataRequest(bool hemisphere) {return TriggerToGate_instance[hemisphere].OnDataRequest();}
void TriggerToGate_OnDataReceive(bool hemisphere, uint32_t data) {TriggerToGate_instance[hemisphere].OnDataReceive(data);}
