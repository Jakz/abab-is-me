#pragma once

#define PLATFORM_DESKTOP 1
#define PLATFORM_OPENDINGUX 2
#define PLATFORM_FUNKEY 3

#if _WIN32
#define TARGET_PLATFORM PLATFORM_DESKTOP
#else
#define TARGET_PLATFORM PLATFORM_OPENDINGUX
#endif

#include <string>

#if TARGET_PLATFORM == PLATFORM_PORTMASTER

  static const std::string DATA_FOLDER = R"(data/Data/)";

#elif TARGET_PLATFORM == PLATFORM_DESKTOP
  
  static const std::string DATA_FOLDER = R"(E:\Games\Steam\steamapps\common\Baba Is You\Data\)";

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
