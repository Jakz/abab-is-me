#pragma once

#include <cstdio>
#include <cstdint>

#include <string>
#include <vector>
#include <memory>

struct Surface;

class AssetLoader
{
protected:
  FILE* file;

  std::string _path;
  std::vector<uint32_t> offsets;

  template<typename T> T read() { T v; fread(&v, sizeof(T), 1, file); return v; }
  auto tell() const { return ftell(file); }

protected:

  bool tryExtractImage(uint32_t offset, const std::string& folder, size_t index);
  bool tryExtractSound(uint32_t offset, const std::string& out_path);
  bool tryExtractShader(uint32_t offset, const std::string& vert_path, const std::string& frag_path);

  bool decode(const std::string& assets, const std::string& outFolder);

public:
  void setPath(const std::string& path) { _path = path; }
  
  void cacheOffsets();
  std::vector<uint8_t> loadSound(uint32_t index);
  Surface* loadImage(uint32_t index);
};
