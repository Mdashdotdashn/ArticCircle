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

class IPropertyBundle
{
public:
  using Position = struct { int x; int y;};

  virtual detail::IProperty& GetProperty() = 0;
  virtual detail::IStringConverter& GetStringConverter() = 0;
  virtual Position& GetPosition() = 0;
};

template <class Property>
class PropertyBundle: public IPropertyBundle
{
public:
  PropertyBundle()
  : stringConverter_(property_)
  {}

  detail::IProperty& GetProperty() override { return property_;}
  detail::IStringConverter& GetStringConverter() override { return stringConverter_;}
  Position& GetPosition() { return position_;}

private:
  Property property_;
  using Converter = string_converter_t<Property>;
  Converter stringConverter_;
  Position position_;
};

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
  IPropertyBundle& GetBundle(int index)
  {
    return *bundles_[index];
  }

  // Property based accessor
  template <class Property>
  IPropertyBundle& GetBundle()
  {
    constexpr auto index = detail::Index<Property, std::tuple<Props...>>::value;
    return GetBundle(index);
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
  auto& GetBundle(int index)
  {
    return properties_.GetBundle(index);
  }

  auto& GetProperty(int index)
  {
    return GetBundle(index).GetProperty();
  }

  auto& GetStringConverter(int index)
  {
    return GetBundle(index).GetStringConverter();
  }

  auto& GetPosition(int index)
  {
    return GetBundle(index).GetPosition();
  }

  void SetPosition(int index, int x, int y)
  {
    auto& position = GetPosition(index);
    position.x = x;
    position.y = y;
  }

  // Type based access

  template <typename Property>
  auto& GetBundle()
  {
    return properties_.template GetBundle<Property>();
  }

  template <typename Property>
  Property& GetProperty()
  {
    auto &bundle = GetBundle<Property>();
    auto &property = bundle.GetProperty();
    return static_cast<Property&>(property);
  }

  template <typename Property>
  void SetPosition(int x, int y)
  {
    auto& position = GetBundle<Property>().GetPosition();
    position.x = x;
    position.y = y;
  }

  //

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
    GetProperty(cursor_).update(direction);
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
  Property& Get()
  {
    return propertyManager_. template GetProperty<Property>();
  }

  template <class Property>
  void setCallback(const auto& cb)
  {
    Get<Property>().setCallback(cb);
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
    propertyManager_.template SetPosition<Property>(x, y);
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
      propertyManager_.SetPosition(index, x, y);
    }
  }

  virtual void drawApplet()
  {
    const auto cursor = propertyManager_.cursor();
    const auto xOffset = 1;
    const auto yOffset = 15;

    for (std::size_t index = 0; index < propertyManager_.size(); index++)
    {
      const auto position = propertyManager_.GetPosition(index);
      const auto stringRender = propertyManager_.GetStringConverter(index).Render();
      gfxPrint(xOffset + position.x , yOffset + position.y, stringRender.c_str());
      if (index == cursor)
      {
        gfxInvert(xOffset + position.x, yOffset + position.y-1, stringRender.length() * 6 + 1, 9);
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
