#pragma once

#include "AssetCache.h"

namespace gfx
{
  class Backend
  {
    
  };
  
  class Gfx
  {
  protected:
    AssetCache _cache;

  public:
    Gfx() : renderer(nullptr) { }
    
    AssetCache* cache() { return &_cache; }
    SDL_Renderer* renderer;
    
    static Gfx i;
  };
}