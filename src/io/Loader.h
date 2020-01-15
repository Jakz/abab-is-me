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

    void loadLD(const path& path, baba::Level* level, baba::GameData& data);

  public:
    Loader(baba::GameData& data);

    baba::Level* load(const std::string& name);
    

    void loadGameData();
  };
}