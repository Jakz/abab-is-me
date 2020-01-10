#pragma once

#include "Types.h"

#include <unordered_map>

namespace baba
{
  enum class ObjectProperty
  {
    YOU = 0x00000001
  };

  using ObjectProperties = bit_mask<ObjectProperty>;

  struct ObjectState
  {
    ObjectProperties properties;
  };

  struct Rules
  {
  private:
    std::unordered_map<const ObjectSpec*, ObjectState> _state;

  public:
    ObjectState& state(const ObjectSpec* spec) { return _state[spec]; }
  };

}