#pragma once

#include "Types.h"

namespace baba
{
  struct Level
  {
  private:
    coord_t _width, _height;
    std::vector<Tile> _tiles;

  public:
    Level(coord_t width, coord_t height) : _width(width), _height(height)
    {
      _tiles.resize(width*height);
      for (coord_t y = 0; y < height; ++y)
        for (coord_t x = 0; x < width; ++x)
          get(x, y)->coord = { x, y };
    }

    auto begin() const { return _tiles.begin(); }
    auto end() const { return _tiles.end(); }

    auto begin() { return _tiles.begin(); }
    auto end() { return _tiles.end(); }

    Tile* get(coord_t i) { return &_tiles[i]; }

    Tile* get(coord_t x, coord_t y) { return x >= 0 && x < _width && y >= 0 && y < _height ? &_tiles[y*_width + x] : nullptr; }
    const Tile* get(coord_t x, coord_t y) const { return x >= 0 && x < _width && y >= 0 && y < _height ? &_tiles[y*_width + x] : nullptr; }

    Tile* get(Tile* t, D d);

    void computeTiling();

    coord_t height() const { return _height; }
    coord_t width() const { return _width; }
  };
}
