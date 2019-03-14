#pragma once

#include "property.h"
#include "string_conversion.h"
#include "value_conversion.h"

//------------------------------------------------------------------------------

struct IPropertyBundle
{
public:
  using Position = struct { int x; int y;};

  virtual detail::IProperty& getProperty() = 0;
  virtual detail::IStringConverter& getStringConverter() = 0;
  virtual detail::IValueConverter& getValueConverter() = 0;

  Position position;
  bool visibility = true;
};

template <class Property>
class PropertyBundle: public IPropertyBundle
{
public:
  PropertyBundle()
  : stringConverter_(property_)
  , valueConverter_(property_)
  {}

  detail::IProperty& getProperty() override { return property_;}
  detail::IStringConverter& getStringConverter() override { return stringConverter_;}
  detail::IValueConverter& getValueConverter() override { return valueConverter_;}

private:
  Property property_;
  using StringConverter = string_converter_t<Property>;
  using ValueConverter = value_converter_t<Property>;
  StringConverter stringConverter_;
  ValueConverter valueConverter_;
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
    auto& converter = bundle.getValueConverter();
    converter.update(direction);
  }

  void setVisibility(IPropertyBundle& bundle, bool visible)
  {
    bundle.visibility = visible;
  }
}
