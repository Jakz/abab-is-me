#pragma once

#include <cassert>
#include <string>

template<typename T>
struct bit_mask
{
  using utype = typename std::underlying_type<T>::type;
  utype value;

  inline bool isSet(T flag) const { return value & static_cast<utype>(flag); }
  inline void set(T flag) { value |= static_cast<utype>(flag); }
  inline void reset(T flag) { value &= ~static_cast<utype>(flag); }
  inline void set(T flag, bool value) { if (value) set(flag); else reset(flag); }

  inline bit_mask<T> operator~() const
  {
    return bit_mask<T>(~value);
  }

  inline bit_mask<T> operator&(T flag) const
  {
    return bit_mask<T>(value & static_cast<utype>(flag));

  }

  inline bit_mask<T> operator|(T flag) const
  {
    return bit_mask<T>(value | static_cast<utype>(flag));
  }

  inline bit_mask<T> operator&(const bit_mask<T>& other) const
  {
    return bit_mask<T>(value & other.value);
  }

  bit_mask<T>() : value(0) { }

private:
  bit_mask<T>(utype value) : value(value) { }
};

using path = std::string;

static const path DATA_FOLDER = R"(E:\Games\Portable\Baba.Is.You.v01.08.2019\Data\)";

constexpr int32_t WIDTH = 1024;
constexpr int32_t HEIGHT = 768;

namespace baba
{
  struct ObjectSpec;
  struct Level;
  struct GameData;
}