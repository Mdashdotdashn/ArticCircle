#pragma once

#include "property_set.h"

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
    const auto current = cursor_;
    const auto incCursor = [this] {
      this->cursor_ = (this->cursor_ + 1) % this->size();
    };

    incCursor();
    while (cursor_ != current)
    {
      auto& bundle = getBundle(cursor_);
      if (bundle.visibility) { break; }
      incCursor();
    }
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
