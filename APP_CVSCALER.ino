// Copyright (c) 2018, Jason Justian
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

#include "HSApplication.h"
#include "src/nostromo/cvscaler/scale_curve.h"

#include <array>

class CVScaler : public HSApplication {
public:
    void Start() {
        Resume();
    }

    void Resume() {
      for (byte ch = 0; ch < 4; ch++)
      {
        curves[ch].reset(-HSAPPLICATION_3V, HSAPPLICATION_5V);
      }
      updateOutput();
    }

    void Controller() {

      if (Changed(0))
      {
        updateOutput();
      }

      for (byte ch = 0; ch < 4; ch++)
      {
        Out(ch, currentCvOut[ch]);
      }
    }

    void View() {
        gfxHeader("CV Scaler");
        gfxPrint(15, 15, "I ");
        gfxPrintVoltage(currentCvIn);
        gfxInvert(15, 15, 7, 9);

        for (byte ch = 0; ch < 4; ch++)
        {
          int x = 64;
          int y = 15 + 10 * ch;
          gfxPrint(x, y, ch);
          gfxPrint(" ");
          if (ch == cursor) gfxInvert(x, y, 7, 9);
          gfxPrintVoltage(currentCvOut[ch]);
          gfxPrint(0,55, debugString.c_str());
        }
    }


    /////////////////////////////////////////////////////////////////
    // Control handlers
    /////////////////////////////////////////////////////////////////
    void OnLeftButtonPress() {
    }

    void OnLeftButtonLongPress() {
    }

    void OnRightButtonPress() {
    }

    void OnUpButtonPress() {
    }

    void OnDownButtonPress() {
    }

    void OnDownButtonLongPress() {
    }

    void OnLeftEncoderMove(int direction) {
      cursor += direction;
      if (cursor < 0) cursor += 4;
      if (cursor > 3) cursor -= 4;
    }

    void OnRightEncoderMove(int direction) {
      const auto target = currentCvOut[cursor] + 30 * direction;
      curves[cursor].update(currentCvIn, target);
      debugString = String("") + currentCvIn + " : " + target;
      currentCvOut[cursor] = curves[cursor].map(currentCvIn);
    }

private:
  void updateOutput()
  {
    currentCvIn = In(0);
    for (byte ch = 0; ch < 4; ch++)
    {
      currentCvOut[ch] = curves[ch].map(currentCvIn);
    }
  }
private:
  std::array<ScaleCurve, 4> curves;
  int cursor = 0;
  int currentCvOut[4];
  int currentCvIn;
  String debugString;
};

CVScaler CVScaler_instance;

// App stubs
void CVScaler_init() {
    CVScaler_instance.BaseStart();
}

// Not using O_C Storage
size_t CVScaler_storageSize() {return 0;}
size_t CVScaler_save(void *storage) {return 0;}
size_t CVScaler_restore(const void *storage) {return 0;}

void CVScaler_isr() {
    return CVScaler_instance.BaseController();
}

void CVScaler_handleAppEvent(OC::AppEvent event) {
    if (event ==  OC::APP_EVENT_RESUME) {
        CVScaler_instance.Resume();
    }
}

void CVScaler_loop() {} // Deprecated

void CVScaler_menu() {
    CVScaler_instance.BaseView();
}

void CVScaler_screensaver() {} // Deprecated

void CVScaler_handleButtonEvent(const UI::Event &event) {
    // For left encoder, handle press and long press
    if (event.control == OC::CONTROL_BUTTON_L) {
        if (event.type == UI::EVENT_BUTTON_LONG_PRESS) CVScaler_instance.OnLeftButtonLongPress();
        else CVScaler_instance.OnLeftButtonPress();
    }

    // For right encoder, only handle press (long press is reserved)
    if (event.control == OC::CONTROL_BUTTON_R && event.type == UI::EVENT_BUTTON_PRESS) CVScaler_instance.OnRightButtonPress();

    // For up button, handle only press (long press is reserved)
    if (event.control == OC::CONTROL_BUTTON_UP) CVScaler_instance.OnUpButtonPress();

    // For down button, handle press and long press
    if (event.control == OC::CONTROL_BUTTON_DOWN) {
        if (event.type == UI::EVENT_BUTTON_PRESS) CVScaler_instance.OnDownButtonPress();
        if (event.type == UI::EVENT_BUTTON_LONG_PRESS) CVScaler_instance.OnDownButtonLongPress();
    }
}

void CVScaler_handleEncoderEvent(const UI::Event &event) {
    // Left encoder turned
    if (event.control == OC::CONTROL_ENCODER_L) CVScaler_instance.OnLeftEncoderMove(event.value);

    // Right encoder turned
    if (event.control == OC::CONTROL_ENCODER_R) CVScaler_instance.OnRightEncoderMove(event.value);
}
