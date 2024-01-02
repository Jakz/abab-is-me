#pragma once

#include "Common.h"

#include <unordered_map>
#include <numeric>
#include <memory>


#include <array>

struct Palette
{
  static constexpr int32_t W = 7, H = 5;
  using data_t = std::array<color_t, 35>;
  
  data_t _data;

  Palette(const std::array<color_t, 35>& data) : _data(data) { }

  const color_t& operator[](int32_t index) const { return _data[index]; }
  const color_t& at(int32_t index) const { return _data[index]; }
  const color_t& at(int32_t x, int32_t y) const { return _data[y * W + x]; }

};

class AssetCache
{
private:
  static constexpr uint32_t FRAMES = 3;

  mutable std::unordered_map<const baba::ObjectSpec*, std::unique_ptr<Texture>> _objectGfxs;
  mutable std::unordered_map<std::string, std::unique_ptr<Texture>> _imageGfxs;
  mutable std::unordered_map<const baba::Icon*, std::unique_ptr<Texture>> _iconGfxs;
  
  mutable std::unordered_map<std::string, std::unique_ptr<Palette>> _palettes;

  path _dataFolder;

public:
  const Texture* objectGfx(const baba::ObjectSpec* spec) const;
  const Texture* imageGfx(const std::string& image) const;
  const Texture* iconGfx(const baba::Icon* spec) const;

  const Palette* palette(const std::string& name) const;

  void flushCache();

public:
  AssetCache();

  void loadPalettes();

  void setDataFolder(const path& dataFolder) { _dataFolder = dataFolder; }
};