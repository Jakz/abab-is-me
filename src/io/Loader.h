#pragma once

#include "Common.h"

namespace io
{
  struct TempData;
  
  class Loader
  {
  private:
    FILE* in;

    baba::Level* readLayer(uint16_t version, baba::Level* level);

    void loadLD(const path& path, baba::Level* level, TempData& tempData, bool headerOnly = false);

  public:
    Loader();

    baba::Level* load(const std::string& name, const baba::GameData& baseData);
    baba::GameData loadGameData();
  };
}