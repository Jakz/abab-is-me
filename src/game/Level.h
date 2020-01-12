#pragma once

#include "Types.h"
#include "Tile.h"

namespace baba
{
  using LevelState = std::vector<Tile>;

  struct Level
  {
  private:
    coord_t _width, _height;
    LevelState _tiles;

  public:
    Level(coord_t width, coord_t height) : _width(width), _height(height)
    {
      _tiles.resize(width*height);
      for (coord_t y = 0; y < height; ++y)
        for (coord_t x = 0; x < width; ++x)
          get(x, y)->coord = { x, y };
    }

    decltype(_tiles)::const_iterator begin() const { return _tiles.begin(); }
    decltype(_tiles)::const_iterator end() const { return _tiles.end(); }

    decltype(_tiles)::iterator begin() { return _tiles.begin(); }
    decltype(_tiles)::iterator end() { return _tiles.end(); }

    Tile* get(coord_t i) { return &_tiles[i]; }

    Tile* get(coord_t x, coord_t y) { return x >= 0 && x < _width && y >= 0 && y < _height ? &_tiles[y*_width + x] : nullptr; }
    const Tile* get(coord_t x, coord_t y) const { return x >= 0 && x < _width && y >= 0 && y < _height ? &_tiles[y*_width + x] : nullptr; }

    Tile* get(const Tile* t, D d);
    const Tile* get(const Tile* t, D d) const;

    void forEachObject(std::function<void(Object&)> lambda);

    void computeTiling();

    coord_t height() const { return _height; }
    coord_t width() const { return _width; }
  };
}
