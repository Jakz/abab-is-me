#pragma once

#include <cstdio>
#include <cstdint>

#include <string>
#include <vector>

class AssetLoader
{
protected:
  FILE* file;

  std::vector<uint32_t> offsets;

  template<typename T> T read() { T v; fread(&v, sizeof(T), 1, file); return v; }
  auto tell() const { return ftell(file); }

protected:
  void cacheOffsets();

  bool tryExtractImage(uint32_t offset, const std::string& folder, size_t index);
  bool tryExtractSound(uint32_t offset, const std::string& out_path);
  bool tryExtractShader(uint32_t offset, const std::string& vert_path, const std::string& frag_path);

public:
  bool decode(const std::string& assets, const std::string& outFolder);
};
