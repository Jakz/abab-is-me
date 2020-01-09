#pragma once

#include "Common.h"

namespace baba
{
  struct Level;
  struct GameData;
}

namespace io
{
  class Loader
  {
  private:
    baba::GameData& data;
    FILE* in;

    baba::Level* readLayer(uint16_t version);

  public:
    Loader(baba::GameData& data);

    baba::Level* load(const path& path);

    void loadGameData();
  };
}