#include "Loader.h"

#include "Zip.h"

#include <cstdio>

#define LOGD(x, ...) printf(x ## "\n", __VA_ARGS__)


using namespace io;

Loader::Loader()
{

}

template<typename T> T read(FILE* in) { T t;  fread(&t, sizeof(T), 1, in); return t; }
template<typename T> void read(T& dest, FILE* in) { fread(&dest, sizeof(T), 1, in); }
void skip(size_t count, FILE* in) { fseek(in, count, SEEK_CUR); }
size_t flength(FILE* in) { fseek(in, 0, SEEK_END); size_t ot = ftell(in); fseek(in, 0, SEEK_SET); return ot; }

void Loader::load(const path& path)
{  
  in = fopen(path.c_str(), "rb");

  //TODO: endianness
  static constexpr uint64_t ACHTUNG = 0x21474e5554484341;
  static constexpr uint32_t MAP = 0x2050414d;
  static constexpr uint64_t LAYR = 0x5259414c;

  static constexpr uint16_t MIN_VERSION = 256, MAX_VERSION = 261;

  const size_t length = flength(in);


  uint64_t achtung;
  uint16_t version;

  read(achtung, in);
  assert(achtung == achtung);

  read(version, in);
  assert(version >= MIN_VERSION && version <= MAX_VERSION);

  while (ftell(in) < length)
  {
    uint32_t header;
    read(header, in);
    skip(4, in);

    if (header == MAP)
    {
      skip(2, in);
    }
    else if (header == LAYR)
    {
      uint16_t layerCount = read<uint16_t>(in);
      for (size_t i = 0; i < layerCount; ++i)
        readLayer(version);
    }
    else
      assert(false);

  }

  fclose(in);
}

void Loader::readLayer(uint16_t version)
{
  uint32_t width = read<uint32_t>(in);
  uint32_t height = read<uint32_t>(in);

  LOGD("Reading a layer of %dx%d cells", width, height);

  if (version >= 258) skip(4, in);
  skip(25, in);
  if (version == 260) skip(2, in);
  else if (version == 261) skip(3, in);

  uint8_t dataBlocks = read<uint8_t>(in);
  assert(dataBlocks == 1 || dataBlocks == 2);

  /* MAIN */
  skip(4, in);
  uint32_t compressedSize = read<uint32_t>(in);
  uint32_t nextPosition = ftell(in) + compressedSize;

  Zip::byte* buffer = (Zip::byte*)std::calloc(compressedSize, 1);
  fread(buffer, 1, compressedSize, in);
  auto uncompressed = Zip::uncompress(buffer, compressedSize);


}