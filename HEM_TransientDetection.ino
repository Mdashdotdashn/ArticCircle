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

class TransientDetector : public HemisphereApplet {
public:
    constexpr static float kSampleRate = float(16667);

    const char* applet_name() { // Maximum 10 characters
        return "Transient";
    }

	/* Run when the Applet is selected */
    void Start() {
      state_ = 0;
      peak_ =0;
      release_ = 40;
      updateReleaseCoefficient();
    }

	/* Run during the interrupt service routine, 16667 times per second */
    void Controller() {
      static const sample_t kPeakFall = sample_t(1.f/kSampleRate/20.f);
      static const sample_t kMinimumPeak = sample_t(0.2);
      const auto input = sample_t::fromRatio(In(0), HEMISPHERE_3V_CV);
      const auto rectified = abs(input);
      peak_ = max(peak_, rectified);
      peak_ = max(peak_ - kPeakFall, kMinimumPeak);
      state_ = rectified > state_ ? rectified : state_ + (rectified - state_) * releaseCoeff_;
      Out(0, float(state_ /peak_) * HEMISPHERE_3V_CV);
      Out(1, float(peak_) * HEMISPHERE_3V_CV);
    }

	/* Draw the screen */
    void View() {
        gfxHeader(applet_name());
        gfxSkyline();
        // Add other view code as private methods
        gfxPrint(21 + pad(100, release_), 15, release_);

    }

	/* Called when the encoder button for this hemisphere is pressed */
    void OnButtonPress() {
    }

	/* Called when the encoder for this hemisphere is rotated
	 * direction 1 is clockwise
	 * direction -1 is counterclockwise
	 */
    void OnEncoderMove(int direction) {
      release_ = clamp(release_ + direction, 20, 500);
      updateReleaseCoefficient();
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

  void updateReleaseCoefficient()
  {
    releaseCoeff_ = onePoleCoeff(kSampleRate, float(release_) / 100.f);
  }
 
private:
  sample_t peak_;
  sample_t state_;
  int release_;
  sample_t releaseCoeff_; 
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to TransientDetector,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
TransientDetector TransientDetector_instance[2];

void TransientDetector_Start(bool hemisphere) {TransientDetector_instance[hemisphere].BaseStart(hemisphere);}
void TransientDetector_Controller(bool hemisphere, bool forwarding) {TransientDetector_instance[hemisphere].BaseController(forwarding);}
void TransientDetector_View(bool hemisphere) {TransientDetector_instance[hemisphere].BaseView();}
void TransientDetector_OnButtonPress(bool hemisphere) {TransientDetector_instance[hemisphere].OnButtonPress();}
void TransientDetector_OnEncoderMove(bool hemisphere, int direction) {TransientDetector_instance[hemisphere].OnEncoderMove(direction);}
void TransientDetector_ToggleHelpScreen(bool hemisphere) {TransientDetector_instance[hemisphere].HelpScreen();}
uint32_t TransientDetector_OnDataRequest(bool hemisphere) {return TransientDetector_instance[hemisphere].OnDataRequest();}
void TransientDetector_OnDataReceive(bool hemisphere, uint32_t data) {TransientDetector_instance[hemisphere].OnDataReceive(data);}
