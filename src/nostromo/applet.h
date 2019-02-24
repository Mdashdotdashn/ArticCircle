#pragma once

#include "property.h"
#include "string_conversion.h"

constexpr static float kSampleRate = float(16667);

namespace detail
{
  template <class Interface,template <class...> class Concrete, class... Args>
  auto mkarray()
  {
    std::array<std::unique_ptr<Interface>, sizeof...(Args)> a = { std::make_unique<Concrete<Args>>()... };
    return a;
  };

  // Utilities to convert a property class to an index within the array

  template <class T, class Tuple>
  struct Index;

  template <class T, class... Types>
  struct Index<T, std::tuple<T, Types...>> {
      static const std::size_t value = 0;
  };

  template <class T, class U, class... Types>
  struct Index<T, std::tuple<U, Types...>> {
      static const std::size_t value = 1 + Index<T, std::tuple<Types...>>::value;
  };
} // detail

//------------------------------------------------------------------------------

struct IPropertyBundle
{
public:
  using Position = struct { int x; int y;};

  virtual detail::IProperty& getProperty() = 0;
  virtual detail::IStringConverter& getStringConverter() = 0;

  Position position;
};

template <class Property>
class PropertyBundle: public IPropertyBundle
{
public:
  PropertyBundle()
  : stringConverter_(property_)
  {}

  detail::IProperty& getProperty() override { return property_;}
  detail::IStringConverter& getStringConverter() override { return stringConverter_;}

private:
  Property property_;
  using Converter = string_converter_t<Property>;
  Converter stringConverter_;
};


namespace bundle
{
  void setPosition(IPropertyBundle& bundle, int x, int y)
  {
    bundle.position.x = x;
    bundle.position.y = y;
  }

  void update(IPropertyBundle& bundle, int direction)
  {
    auto& property = bundle.getProperty();
    property.update(direction);
  }
}

//------------------------------------------------------------------------------

template <typename... Props>
class PropertySet
{
public:
  // Construct

  PropertySet()
  {
    bundles_ = detail::mkarray<IPropertyBundle, PropertyBundle, Props...>();
  }

  // Index based accessor
  IPropertyBundle& getBundle(int index)
  {
    return *bundles_[index];
  }

  // Property based accessor
  template <class Property>
  IPropertyBundle& getBundle()
  {
    constexpr auto index = detail::Index<Property, std::tuple<Props...>>::value;
    return getBundle(index);
  }

  constexpr static std::size_t size()
  {
    return sizeof...(Props);
  }

private:
  std::array<std::unique_ptr<IPropertyBundle> ,sizeof...(Props)> bundles_;
};

//------------------------------------------------------------------------------

template <typename Model>
struct PropertyManager
{
  using Properties = typename Model::Properties;

  PropertyManager()
  {
    cursor_ = 0;
  }

  // Index based access
  auto& getBundle(int index)
  {
    return properties_.getBundle(index);
  }

  // Type based access

  template <typename Property>
  auto& getBundle()
  {
    return properties_.template getBundle<Property>();
  }

  inline auto size() const
  {
    return properties_.size();
  }

  void next()
  {
    cursor_ = (cursor_ + 1) % size();
  }

  void updateCurrent(const int direction)
  {
    auto& bundle = getBundle(cursor_);
    bundle::update(bundle, direction);
  }

  std::size_t cursor() const
  {
      return cursor_;
  }

  Properties properties_; // The property set related to the model
  std::size_t cursor_;
};

//------------------------------------------------------------------------------

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
  virtual void layout() {};

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
    tick();
  }

/* Draw the screen */
  void View() {
    gfxHeader(name_);

    ForEachChannel(ch)
    {
      if (Gate(ch))
      {
        const auto offset = 5 * ch;
        gfxRect(58, offset, 4, 4);
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
 };
