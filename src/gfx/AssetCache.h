#pragma once

#include "Common.h"

#include "io/Assets.h"

#include <unordered_map>
#include <numeric>
#include <memory>


#include <array>

using asset_index = int32_t;

struct asset_list : public std::vector<asset_index>
{
  using vector::vector;
  asset_index random() const { return (*this)[rand() % size()]; }
};

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

struct SoundData
{
  std::vector<uint8_t> data;
  SoundNativeData nativeData;

  SoundData() { }
  SoundData(const std::vector<uint8_t>& data) : data(std::move(data)), nativeData(this->data)
  {

  }
};

class SoundDataCache
{
public:
  std::unordered_map<uint32_t, SoundData> _sounds;
};

class AssetCache
{
private:
  static constexpr uint32_t FRAMES = 3;

  mutable std::unordered_map<const baba::ObjectSpec*, std::unique_ptr<Texture>> _objectGfxs;
  mutable std::unordered_map<std::string, std::unique_ptr<Texture>> _imageGfxs;
  mutable std::unordered_map<const baba::Icon*, std::unique_ptr<Texture>> _iconGfxs;

  mutable std::unordered_map<std::string, asset_list> _numberedGfxIndices;
  mutable std::unordered_map<std::string, std::unique_ptr<Texture>> _numberedGfxs;
  
  mutable std::unordered_map<std::string, std::unique_ptr<Palette>> _palettes;

  SoundDataCache _sounds;

  path _dataFolder;

  AssetLoader _loader;

public:
  const Texture* objectGfx(const baba::ObjectSpec* spec) const;
  const Texture* imageGfx(const std::string& image) const;
  const Texture* iconGfx(const baba::Icon* spec) const;

  const Texture* numberedGfx(const std::string& key);

  const Palette* palette(const std::string& name) const;

  const SoundData& sound(uint32_t index);

  void flushCache();
  void loadPalettes();

public:
  AssetCache();

  void init(const path& baseFolder);
};

class AssetMapping
{
public:
  static const asset_list BABA_STEP_SOUND;
  static const asset_list DEFEAT_SOUND;
};