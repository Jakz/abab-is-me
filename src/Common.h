#pragma once

#include <cassert>
#include <string>
#include <vector>
#include "utils/format/format.h"

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

  point_t(coord_t x, coord_t y) : x(x), y(y) { }
  point_t() : point_t(0, 0) { }

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
  size2d_t() : size2d_t(0, 0) { }
  size2d_t(coord_t w, coord_t h) : w(w), h(h) { }
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

#if USE_SDL

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"

#include "gfx/SdlHelper.h"

using color_t = SDL_Color;
using rect_t = SDL_Rect;

using Renderer = SDL;

struct Surface
{
public:
  SDL_Surface* _surface;

  coord_t _width, _height;

public:
  Surface() : _surface(nullptr), _width(0), _height(0) { }
  Surface(SDL_Surface* surface) : _surface(surface), _width(surface->w), _height(surface->h)
  {

  }

  Surface(Surface&& other) noexcept
  {
    _surface = nullptr;
    _width = other._width;
    _height = other._height;
    std::swap(_surface, other._surface);
  }

  Surface& operator=(Surface&& other) noexcept
  {
    _width = other._width;
    _height = other._height;
    std::swap(_surface, other._surface);
    return *this;
  }

  ~Surface()
  {
    destroy();
  }
  
  operator bool() const { return _surface != nullptr; }
  coord_t height() const { return _height; }
  coord_t width() const { return _width; }

  void destroy()
  {
    if (_surface)
      SDL_FreeSurface(_surface);
    _surface = nullptr;
  }
};

struct Texture
{
public:
  SDL_Texture* _texture;

  std::vector<rect_t> _rects;
  coord_t _width, _height;
  coord_t _rows;

public:
  Texture(SDL_Texture* texture, coord_t w, coord_t h) : Texture(texture, size2d_t(w, h), { { 0, 0, w, h } }) { }
  Texture(SDL_Texture* texture, size2d_t size, const std::vector<rect_t>& rects) :
    _texture(texture), _width(size.w), _height(size.h), _rects(rects), _rows(1)
  {

  }

  ~Texture()
  {
    destroy();
  }

  coord_t height() const { return _height; }
  coord_t width() const { return _width; }

  coord_t rows() const { return _rows; }
  coord_t cols() const { return _rects.size() / rows(); }

  size_t count() const { return _rects.size(); }
  const rect_t& rect(size_t index) const { return index < _rects.size() ? _rects[index] : _rects[0]; }
  const rect_t& rect(size_t x, size_t y) const
  {
    size_t index = y * cols() + x;
    return rect(index);
  }

  SDL_Texture* texture() const { return _texture; }

  void setRows(coord_t rows) { _rows = rows; }
  void setRects(const std::vector<rect_t>& rects) { _rects = rects; }
  void enableAlphaBlending() { SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_BLEND); }

  void destroy()
  {
    if (_texture)
      SDL_DestroyTexture(_texture); 
    _texture = nullptr; 
  }
};

struct SoundNativeData
{
  SDL_RWops* _rw;
  Mix_Chunk* _chunk;
  
  SoundNativeData() : _rw(nullptr), _chunk(nullptr) { }
  SoundNativeData(const std::vector<uint8_t>& data) : SoundNativeData()
  {
    _rw = SDL_RWFromConstMem(data.data(), data.size());
    _chunk = Mix_LoadWAV_RW(_rw, false);
  }
  
  ~SoundNativeData()
  {
    if (_rw)
      SDL_RWclose(_rw);
    _rw = nullptr;
    
    if (_chunk)
      Mix_FreeChunk(_chunk);
    _chunk = nullptr;
  }

  SoundNativeData(SoundNativeData&& other) noexcept
  {
    std::swap(_rw, other._rw);
    std::swap(_chunk, other._chunk);
  }

  SoundNativeData& operator=(SoundNativeData&& other) noexcept
  {
    std::swap(_rw, other._rw);
    std::swap(_chunk, other._chunk);
    return *this;
  }

  auto chunk() const { return _chunk; }
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
  
  KeyReturn = SDLK_RETURN,

  KeySpace = SDLK_SPACE,
  KeyEsc = SDLK_ESCAPE,

  BindLeft = KeyCode::KeyA,
  BindRight = KeyCode::KeyD,
  BindUp = KeyCode::KeyW,
  BindDown = KeyCode::KeyS,

  BindWait = KeyCode::KeySpace,
  BindExit = KeyCode::KeyEsc,
  BindUndo = KeyCode::KeyZ,
  BindAction = KeyCode::KeyReturn,
};

#else

#include "libretro.h"

struct color_t
{
  uint8_t r, g, b, a;

  color_t() : r(0), g(0), b(0), a(255) { }
};

struct rect_t
{
  int x, y;
  int w, h;
};

enum class KeyCode
{
  KeyA = RETROK_a,
  KeyD = RETROK_d,
  KeyG = RETROK_g,
  KeyL = RETROK_l,
  KeyM = RETROK_m,
  KeyR = RETROK_r,
  KeyS = RETROK_s,
  KeyW = RETROK_w,
  KeyZ = RETROK_z,

  KeyKpPlus = RETROK_KP_PLUS,
  KeyKpMinus = RETROK_KP_MINUS ,

  KeySpace = RETROK_SPACE,
  KeyEsc = RETROK_ESCAPE,

  BindLeft = KeyCode::KeyA,
  BindRight = KeyCode::KeyD,
  BindUp = KeyCode::KeyW,
  BindDown = KeyCode::KeyS,

  BindWait = KeyCode::KeySpace,
  BindExit = KeyCode::KeyEsc,
  BindUndo = KeyCode::KeyZ,
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


namespace events
{
  struct KeyEvent
  { 
    KeyCode code;
    bool press;
  };

  struct MouseEvent
  {
    int32_t x, y;
  };
}