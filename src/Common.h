#pragma once

#include <cassert>
#include <string>

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