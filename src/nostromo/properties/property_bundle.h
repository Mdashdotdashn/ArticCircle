#pragma once

#include "string_conversion.h"
#include "property.h"

//------------------------------------------------------------------------------

struct IPropertyBundle
{
public:
  using Position = struct { int x; int y;};

  virtual detail::IProperty& getProperty() = 0;
  virtual detail::IStringConverter& getStringConverter() = 0;

  Position position;
  bool visibility = true;
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

  void setVisibility(IPropertyBundle& bundle, bool visible)
  {
    bundle.visibility = visible;
  }
}
