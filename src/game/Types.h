#pragma once

#include "Common.h"

#include <vector>

namespace baba
{
  struct ObjectSpec
  {
    std::string name;
    std::string sprite;
  };

  struct GameData
  {
    std::vector<ObjectSpec> objects;
  };
}
