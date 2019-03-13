#pragma once

#include "property_manager.h"
#include "../ui/trigger_display.h"

constexpr static float kSampleRate = float(16667);

template <class Model>
class ArticCircleApplet: public HemisphereApplet
{
public:
  ArticCircleApplet()
  {
    initLayout();
  }

  virtual void tick() = 0;
  virtual void reset() {};

  void gfxPrintF(int x, int y, float value)
  {
    static char buffer[20];
    snprintf(buffer,20, "%.2f", value);
    gfxPrint(x, y, buffer);
  }

  template <class Property>
  IPropertyBundle& getBundle()
  {
    return propertyManager_. template getBundle<Property>();
  }

  template <class Property>
  Property& getProperty()
  {
    auto& bundle = getBundle<Property>();
    return static_cast<Property&>(bundle.getProperty());
  }

  template <class Property>
  void setCallback(const auto& cb)
  {
    getProperty<Property>().setCallback(cb);
  }

  template <typename Property>
  void bind(auto& v)
  {
    setCallback<Property>([&v](const auto& value)
    {
      v = value;
    });
  }

  template <typename Property>
  void setPosition(int x, int y)
  {
    auto& bundle = getBundle<Property>();
    bundle::setPosition(bundle, x, y);
  }

  template <typename Property>
  void setRange(const typename Property::value_t& min, const typename Property::value_t& max)
  {
    getProperty<Property>().setRange(min, max);
  }

  template <typename Property>
  void setVisibility(bool visible)
  {
    auto& bundle = getBundle<Property>();
    bundle::setVisibility(bundle, visible);
  }

  void setName(const char* name)
  {
    name_ = name;
  }

/* Run when the Applet is selected */
  void Start()
  {
    reset();
  }

/* Run during the interrupt service routine, 16667 times per second */
  void Controller()
  {
    ForEachChannel(ch)
    {
      sizer_[ch].feed(Gate(ch));
    }
    tick();
  }

/* Draw the screen */
  void View() {
    gfxHeader(name_);

    ForEachChannel(ch)
    {
      const auto size = sizer_[ch].updateSize();
      if (size > 0)
      {
        const auto offset = 5 * ch;
        gfxRect(58, offset, size, size);
      }
    }
    drawApplet();
  }

  void initLayout()
  {
    const auto kParameterRow = 6;
    const auto kColSpacing = 9;

    for (std::size_t index = 0; index < propertyManager_.size(); index++)
    {
      const auto x = 32 * (index /kParameterRow);
      const auto y = kColSpacing * (index % kParameterRow);
      auto& bundle = propertyManager_.getBundle(index);
      bundle::setPosition(bundle, x, y);
    }
  }

  virtual void drawApplet()
  {
    const auto cursor = propertyManager_.cursor();
    const auto xOffset = 1;
    const auto yOffset = 15;

    for (std::size_t index = 0; index < propertyManager_.size(); index++)
    {
      auto& bundle = propertyManager_.getBundle(index);
      if (bundle.visibility)
      {
        const auto stringRender = bundle.getStringConverter().Render();
        const auto x = xOffset + bundle.position.x;
        const auto y = yOffset + bundle.position.y;
        gfxPrint(x , y, stringRender.c_str());
        if (index == cursor)
        {
          gfxInvert(x, y - 1, stringRender.length() * 6 + 1, 9);
        }
      }
    }
  }

  void OnButtonPress() {
    propertyManager_.next();
  }

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

// retro-compatibility

  const char *applet_name() { return name_; };

  PropertyManager<Model> propertyManager_;
  const char *name_ = "unknown";
  TriggerSizer<4> sizer_[2];
 };
