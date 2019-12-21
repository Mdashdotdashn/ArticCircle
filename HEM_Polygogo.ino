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

#include "braids_quantizer.h"
#include "braids_quantizer_scales.h"
#include "OC_scales.h"

#include <cmath>

namespace NPolygogo
{
  struct Model
  {
    struct Order: Property<float>
    {
      Order()
      {
        setRange(2.1,7,0.1);
        setValue(4);
      }
    };
    using RootNote = RootNoteProperty;
    using Scale = ScaleProperty;
    using Octave = OctaveProperty;

    using Properties = PropertySet<Scale, RootNote, Octave, Order>;
  };

  class Renderer
  {
  public:
    Renderer()
    {
      setOrder(10.f);
    }

    void setOrder(float order)
    {
      // computes the angle size of a slice
      mModulo = 0.5f / order;
      // computes the apothema
      mApothema = quadraticCosine(sample_t(mModulo));
    }

    sample_t computeQuadrantAngle(const sample_t& phasor)
    {
      const float fPhasor = float(phasor);
      const int quadrant = int(floor(fPhasor / mModulo));
      float angle = fPhasor - mModulo * quadrant;
      if (quadrant % 2 == 1)
      {
        // flip angle
        angle = mModulo - angle;
      }
      return sample_t(angle);
    }

    sample_t render(const sample_t& phasor)
    {
      const auto angle = computeQuadrantAngle(phasor);
      // computes the length to the polygon's edge
      const auto hypothenuse = mApothema / quadraticCosine(angle);
      // output the 2 waves
      return hypothenuse * quadraticSine(phasor);
    }
  private:
    sample_t mApothema;
    float mModulo;
  };

  class Applet : public ArticCircleApplet<Model> {
  public:
    Applet()
    {
      // Maximum 9 characters
      //       123456789
      setName("Poly-gogo");

      root_ = 0;
      quantizer_.Init();
      lastNote_ = 0;

      osc_.setTicker([this](const sample_t& phase, const sample_t& /*phaseInc*/)
      {
        return renderer_.render(phase);
      });

      setCallback<Model::Scale>([this](const auto& scale){
        quantizer_.Configure(OC::Scales::GetScale(scale), 0xffff);
      });

      bind<Model::RootNote>(root_);

      setCallback<Model::Octave>([this](const auto& o){
        octave_ = o;
      });

      setCallback<Model::Order>([this](const auto& o){
        mOrder = o;
      });

      renderer_.setOrder(mOrder);
    }

    virtual void reset() final {};

    virtual void tick() final
    {
      const int32_t pitch = In(0);
      const int32_t modulation = In(1);
      if (modulation != lastModulation_)
      {
        const float order = clamp(mOrder + float(Proportion(In(1), HEMISPHERE_MAX_CV, 300))/100.f, 2.02f, 10.f);
        renderer_.setOrder(order);        
      }
      const int32_t quantized = quantizer_.Process(pitch, root_ << 7, 0);
      lastNote_ = clamp(MIDIQuantizer::NoteNumber(quantized) -24 + octave_ * 12, 0 , 127);
      const float frequency = midiNoteToFrequency(lastNote_) ;
      osc_.setFrequency(frequency);
      const auto output = float(osc_.tick()) * HEMISPHERE_3V_CV;
      Out(0, output);
      Out(1, output);
    }

    void drawApplet() final
    {
      ArticCircleApplet<NPolygogo::Model>::drawApplet();
    }

  private:
    Oscillator<sample_t> osc_;
    Renderer renderer_;
    braids::Quantizer quantizer_;
    float mOrder;
    int32_t lastModulation_ = 0;
    int32_t lastNote_;
    int root_;
    int octave_;
  };

  Applet instance_[2];

} // NPolygogo

void Polygogo_Start(bool hemisphere) {NPolygogo::instance_[hemisphere].BaseStart(hemisphere);}
void Polygogo_Controller(bool hemisphere, bool forwarding) {NPolygogo::instance_[hemisphere].BaseController(forwarding);}
void Polygogo_View(bool hemisphere) {NPolygogo::instance_[hemisphere].BaseView();}
void Polygogo_OnButtonPress(bool hemisphere) {NPolygogo::instance_[hemisphere].OnButtonPress();}
void Polygogo_OnEncoderMove(bool hemisphere, int direction) {NPolygogo::instance_[hemisphere].OnEncoderMove(direction);}
void Polygogo_ToggleHelpScreen(bool hemisphere) {NPolygogo::instance_[hemisphere].HelpScreen();}
uint32_t Polygogo_OnDataRequest(bool hemisphere) {return NPolygogo::instance_[hemisphere].OnDataRequest();}
void Polygogo_OnDataReceive(bool hemisphere, uint32_t data) {NPolygogo::instance_[hemisphere].OnDataReceive(data);}
