#pragma once

#include "Common.h"

namespace io
{
  class Loader
  {
  private:
    FILE* in;

    void readLayer(uint16_t version);

  public:
    Loader();

    void load(const path& path);
  };
}