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

class ProbabilisticGate : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "Funkitus";
    }

	/* Run when the Applet is selected */
    void Start()
    {
      probability_ = 50;
      state_[0] = state_[1] = false;
      lastGate_[0] = lastGate_[1] = false;
    }

	/* Run during the interrupt service routine, 16667 times per second */
    void Controller() {

      const auto processFlank = [&](int ch, bool up) -> void
      {
        if (up)
        {
          const auto p = (ch == 0) ? probability_ : 100 - probability_;
          if (random(1,100) <= p)
          {
            state_[ch] = true;
          }
        }
        else
        {
          state_[ch] = false;
        }
      };

      ForEachChannel(ch)
      {
        bool gate = Gate(ch);
        if (gate != lastGate_[ch])
        {
          processFlank(ch, gate);
          lastGate_[ch] = gate;
        }
      }

      GateOut(0, state_[0] | state_[1]);
      GateOut(1, state_[0] ^ state_[1]);

    }

	/* Draw the screen */
    void View() {
        gfxHeader(applet_name());
        gfxSkyline();
        // Add other view code as private methods
        gfxPrint(1, 15, "p=");
        gfxPrint(15 + pad(100, probability_), 15, probability_);
    }

	/* Called when the encoder button for this hemisphere is pressed */
    void OnButtonPress() {
    }

	/* Called when the encoder for this hemisphere is rotated
	 * direction 1 is clockwise
	 * direction -1 is counterclockwise
	 */
    void OnEncoderMove(int direction) {
      probability_ = constrain(probability_ += direction, 0, 100);
    }

    /* Each applet may save up to 32 bits of data. When data is requested from
     * the manager, OnDataRequest() packs it up (see HemisphereApplet::Pack()) and
     * returns it.
     */
    uint32_t OnDataRequest() {
      uint32_t data = 0;
      Pack(data, PackLocation {0,7}, probability_);
      return data;
    }

    /* When the applet is restored (from power-down state, etc.), the manager may
     * send data to the applet via OnDataReceive(). The applet should take the data
     * and unpack it (see HemisphereApplet::Unpack()) into zero or more of the applet's
     * properties.
     */
    void OnDataReceive(uint32_t data) {
      probability_ = Unpack(data, PackLocation {0,7});
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
  int probability_;
  bool state_[2];
  bool lastGate_[2];
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to ProbabilisticGate,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
ProbabilisticGate ProbabilisticGate_instance[2];

void ProbabilisticGate_Start(bool hemisphere) {ProbabilisticGate_instance[hemisphere].BaseStart(hemisphere);}
void ProbabilisticGate_Controller(bool hemisphere, bool forwarding) {ProbabilisticGate_instance[hemisphere].BaseController(forwarding);}
void ProbabilisticGate_View(bool hemisphere) {ProbabilisticGate_instance[hemisphere].BaseView();}
void ProbabilisticGate_OnButtonPress(bool hemisphere) {ProbabilisticGate_instance[hemisphere].OnButtonPress();}
void ProbabilisticGate_OnEncoderMove(bool hemisphere, int direction) {ProbabilisticGate_instance[hemisphere].OnEncoderMove(direction);}
void ProbabilisticGate_ToggleHelpScreen(bool hemisphere) {ProbabilisticGate_instance[hemisphere].HelpScreen();}
uint32_t ProbabilisticGate_OnDataRequest(bool hemisphere) {return ProbabilisticGate_instance[hemisphere].OnDataRequest();}
void ProbabilisticGate_OnDataReceive(bool hemisphere, uint32_t data) {ProbabilisticGate_instance[hemisphere].OnDataReceive(data);}
