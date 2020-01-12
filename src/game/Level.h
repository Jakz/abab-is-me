#pragma once

#include "Types.h"
#include "Tile.h"

namespace baba
{
  using LevelState = std::vector<Tile>;

  struct Level
  {
  private:
    const GameData& _data;
    coord_t _width, _height;
    LevelState _tiles;

  public:
    Level(const GameData& data, coord_t width, coord_t height) : _width(width), _height(height), _data(data)
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
    void placeEdge();

    coord_t height() const { return _height; }
    coord_t width() const { return _width; }

    //TODO: use std::move from History class to make it much more efficient
    void restore(const LevelState& state) { _tiles = state; }
    LevelState state() const { return _tiles; }
  };
}

#include <deque>

namespace baba
{
  class History
  {
  private:
    static constexpr size_t MAX_SIZE = 16;
    std::deque<LevelState> history;
  public:
    void push(LevelState&& state)
    {
      history.push_back(state);
      if (history.size() > MAX_SIZE)
        history.pop_front();
    }

    LevelState pop()
    {
      LevelState state = history.back();
      history.pop_back();
      return state;
    }

    bool empty() const { return history.empty(); }
    size_t size() const { return history.size(); }
  };
}

