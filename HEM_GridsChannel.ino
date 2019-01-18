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

#include "grids.h"

class GridsChannel : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "Grids";
    }

	/* Run when the Applet is selected */
    void Start() {
      x_ = 23;
      y_ = 128;
      density_ = 128;
      mode_ = 0;
      cursor_ = 0;
      channel_.reset();
    }

    void processStep()
    {
        const auto density =  constrain(density_ + Proportion(DetentedIn(0), HEMISPHERE_MAX_CV, 256), 0, 256);
        const uint8_t threshold = ~density;

        if (mode_ < 3) // Single
        {
          const auto level = channel_.level(mode_, x_, y_);
          if (level > threshold)
          {
            ClockOut(0); // trigger
            Out(1, Proportion(level - threshold, 256 - threshold, HEMISPHERE_MAX_CV));
          }          
        }
        else // dual
        {
          uint8_t left = mode_ < 5 ? 0 : 1; // B/B/S
          uint8_t right = mode_ < 4 ? 1 : 2; // S/H/H
          
          uint8_t levels[2] =
            {
              channel_.level(left, x_, y_),
              channel_.level(right, x_, y_)
            };

          if (density == 255) // Output levels
          {
            Out(0, Proportion(levels[0], 256, HEMISPHERE_MAX_CV));
            Out(1, Proportion(levels[1], 256, HEMISPHERE_MAX_CV));
          }
          else // Output trigger
          {
            if (levels[0] > threshold) ClockOut(0);
            if (levels[1] > threshold) ClockOut(1);
          }
        }      
    }
  
	/* Run during the interrupt service routine, 16667 times per second */
    void Controller() {

      if (Clock(1))
      {
        channel_.reset();
      }

      if (Clock(0))
      {
        processStep();
        channel_.advance();
      }

    }

	/* Draw the screen */
    void View() {

      gfxHeader(applet_name());

      ForEachChannel(ch)
      {
        if (Gate(ch))
        {
          const auto offset = 5 * ch;
          gfxRect(58, offset, 4, 4);
        }
      }
      
      gfxSkyline();
      // Add other view code as private methods
      gfxPrint(21 + pad(100, density_), 15, density_);
      if (CursorBlink() && cursor_ == 0) gfxLine(21 + 20, 15, 21 + 20, 21);

      static const char *modes[] = { "BD", "SD", "HH", "BS", "BH", "SH" };
      gfxPrint(21 + 6, 25, modes[mode_]);
      if (CursorBlink() && cursor_ == 1) gfxLine(21 + 20, 25, 21 + 20, 31);

      gfxPrint(21 + pad(100, x_), 35, x_);
      if (CursorBlink() && cursor_ == 2) gfxLine(21 + 20, 35, 21 + 20, 41);

      gfxPrint(21 + pad(100, y_), 45, y_);
      if (CursorBlink() && cursor_ == 3) gfxLine(21 + 20, 45, 21 + 20, 51);

    }

	/* Called when the encoder button for this hemisphere is pressed */
    void OnButtonPress() {
      cursor_ = (cursor_ + 1) % 4;
    }

	/* Called when the encoder for this hemisphere is rotated
	 * direction 1 is clockwise
	 * direction -1 is counterclockwise
	 */
    void OnEncoderMove(int direction) {
      if (cursor_ == 0) density_ = constrain(density_ + direction, 0, 255);
      if (cursor_ == 1) mode_ = (mode_ + direction + 6) % 6;
      if (cursor_ == 2) x_ = constrain(x_ + direction, 0, 255);
      if (cursor_ == 3) y_ = constrain(y_ + direction, 0, 255);
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
        help[HEMISPHERE_HELP_DIGITALS] = "0: trig 1: reset";
        help[HEMISPHERE_HELP_CVS]      = "CV in help";
        help[HEMISPHERE_HELP_OUTS]     = "Out help";
        help[HEMISPHERE_HELP_ENCODER]  = "123456789012345678";
        //                               "------------------" <-- Size Guide
    }

private:
    uint8_t x_;
    uint8_t y_;
    uint8_t density_;
    int8_t mode_;

    uint8_t cursor_;
    grids::Channel channel_;
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to GridsChannel,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
GridsChannel GridsChannel_instance[2];

void GridsChannel_Start(bool hemisphere) {GridsChannel_instance[hemisphere].BaseStart(hemisphere);}
void GridsChannel_Controller(bool hemisphere, bool forwarding) {GridsChannel_instance[hemisphere].BaseController(forwarding);}
void GridsChannel_View(bool hemisphere) {GridsChannel_instance[hemisphere].BaseView();}
void GridsChannel_OnButtonPress(bool hemisphere) {GridsChannel_instance[hemisphere].OnButtonPress();}
void GridsChannel_OnEncoderMove(bool hemisphere, int direction) {GridsChannel_instance[hemisphere].OnEncoderMove(direction);}
void GridsChannel_ToggleHelpScreen(bool hemisphere) {GridsChannel_instance[hemisphere].HelpScreen();}
uint32_t GridsChannel_OnDataRequest(bool hemisphere) {return GridsChannel_instance[hemisphere].OnDataRequest();}
void GridsChannel_OnDataReceive(bool hemisphere, uint32_t data) {GridsChannel_instance[hemisphere].OnDataReceive(data);}
