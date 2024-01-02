#pragma once

#include <cassert>
#include <string>
#include <vector>

#define LOGD(x, ...) printf(x "\n", __VA_ARGS__)
#define LOGDD(x) printf(x "\n")

template<typename T>
struct bit_mask
{
  using utype = typename std::underlying_type<T>::type;
  utype value;

  inline void clear() { value = 0; }

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

using coord_t = int32_t;

struct Palette;

struct point_t
{
  coord_t x, y;

  bool operator==(const point_t& o) const { return x == o.x && y == o.y; }
  bool operator!=(const point_t& o) const { return x != o.x || y != o.y; }

  struct hash
  {
    size_t operator()(const point_t& p) const { return p.y << 16 | p.x; }
  };
};

struct size2d_t
{
  coord_t w, h;
};

namespace baba
{
  struct ObjectSpec;
  struct Level;
  struct GameData;
  struct Rules;
 
  enum class ObjectProperty : uint64_t;

  struct Icon;
  struct ObjectGfx;
}

namespace io
{
  class Loader;
}

bool operator&&(const baba::ObjectSpec* spec, baba::ObjectProperty prop);


constexpr int32_t WIDTH = 1024;
constexpr int32_t HEIGHT = 768;

#if USE_SDL

#include "SDL.h"
#include "SDL_image.h"

using color_t = SDL_Color;
using rect_t = SDL_Rect;

struct Texture
{
public:
  SDL_Texture* _texture;

  std::vector<rect_t> _rects;
  coord_t _width, _height;

public:
  Texture(SDL_Texture* texture, size2d_t size, const std::vector<rect_t>& rects) :
    _texture(texture), _width(size.w), _height(size.h), _rects(rects)
  {

  }

  ~Texture()
  {
    destroy();
  }

  coord_t height() const { return _height; }
  coord_t width() const { return _width; }

  size_t count() const { return _rects.size(); }
  const rect_t& rect(size_t index) const { return _rects[index]; }

  SDL_Texture* texture() const { return _texture; }

  void destroy()
  {
    if (_texture)
      SDL_DestroyTexture(_texture); 
    _texture = nullptr; 
  }
};

enum class KeyCode
{
  KeyA = SDLK_a,
  KeyD = SDLK_d,
  KeyG = SDLK_g,
  KeyL = SDLK_l,
  KeyM = SDLK_m,
  KeyR = SDLK_r,
  KeyS = SDLK_s,
  KeyW = SDLK_w,
  KeyZ = SDLK_z,

  KeyKpPlus = SDLK_KP_PLUS,
  KeyKpMinus = SDLK_KP_MINUS,
  
  KeySpace = SDLK_SPACE,
  KeyEsc = SDLK_ESCAPE,

  BindLeft = KeyCode::KeyA,
  BindRight = KeyCode::KeyD,
  BindUp = KeyCode::KeyW,
  BindDown = KeyCode::KeyS,

  BindWait = KeyCode::KeySpace,
  BindExit = KeyCode::KeyEsc,
  BindUndo = KeyCode::KeyZ,
};

#else

struct color_t
{
  uint8_t r, g, b, a;
};

struct rect_t
{
  int x, y;
  int w, h;
};

#endif

enum class KeyBind
{
  Left = KeyCode::KeyA,
  Right = KeyCode::KeyD,
  Up = KeyCode::KeyW,
  Down = KeyCode::KeyS,

  Wait = KeyCode::KeySpace,
  Exit = KeyCode::KeyEsc,
};
