#pragma once

#include "Common.h"

namespace io
{
  class Loader
  {
  private:
    baba::GameData& data;
    FILE* in;

    baba::Level* readLayer(uint16_t version, baba::Level* level);

  public:
    Loader(baba::GameData& data);

    baba::Level* load(const path& path);

    void loadGameData();
  };
}