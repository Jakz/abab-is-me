#pragma once

#define PLATFORM_DESKTOP 1
#define PLATFORM_OPENDINGUX 2
#define PLATFORM_FUNKEY 3

#if _WIN32
#define TARGET_PLATFORM PLATFORM_DESKTOP
#else
#define TARGET_PLATFORM PLATFORM_OPENDINGUX
#endif

#include <SDL.h>
#include <string>

using path = std::string;

static constexpr auto KEY_LEFT = SDLK_LEFT;
static constexpr auto KEY_RIGHT = SDLK_RIGHT;
static constexpr auto KEY_UP = SDLK_UP;
static constexpr auto KEY_DOWN = SDLK_DOWN;
static constexpr auto KEY_WAIT = SDLK_SPACE;

#if TARGET_PLATFORM == PLATFORM_DESKTOP
  
  constexpr int32_t WIDTH = 1024;
  constexpr int32_t HEIGHT = 768;

  static const path DATA_FOLDER = R"(E:\Games\Steam\steamapps\common\Baba Is You\Data\)";

  #define MOUSE_ENABLED true

#elif TARGET_PLATFORM == PLATFORM_OPENDINGUX
  
  constexpr int32_t WIDTH = 320;
  constexpr int32_t HEIGHT = 240;
  
  static const path DATA_FOLDER = R"(/media/sdcard/baba/Data/)";

  #define MOUSE_ENABLED false;

#elif TARGET_PLATFORM == PLATFORM_FUNKEY
  
  constexpr int32_t WIDTH = 240;
  constexpr int32_t HEIGHT = 240;

  #define MOUSE_ENABLED false;

#endif

#if defined(SDL_VERSIONNUM)
  using color_t = SDL_Color;
  using rect_t = SDL_Rect;
#else
  struct color_t
  {
    u8 r, g, b, a;
  };

  struct rect_t
  {
    int x, y;
    int w, h;
  };
#endif