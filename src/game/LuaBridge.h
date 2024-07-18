#pragma once

#include "Common.h"

namespace lua
{
  class LuaBridge
  {
  public:
    void init(const path& dataFolder);
  };
}