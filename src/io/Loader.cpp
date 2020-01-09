#include "Loader.h"

#include "Zip.h"

#include <cstdio>
#include <iostream>
#include <fstream>

#define LOGD(x, ...) printf(x ## "\n", __VA_ARGS__)

using namespace io;

Loader::Loader()
{

}

template<typename T> T read(FILE* in) { T t;  fread(&t, sizeof(T), 1, in); return t; }
template<typename T> void read(T& dest, FILE* in) { fread(&dest, sizeof(T), 1, in); }
void skip(size_t count, FILE* in) { fseek(in, count, SEEK_CUR); }
size_t flength(FILE* in) { fseek(in, 0, SEEK_END); size_t ot = ftell(in); fseek(in, 0, SEEK_SET); return ot; }

/*
  8 bytes ACHTUNG header
  2 bytes version (>= 256, <= 261)

  4 bytes header ("MAP " or "LAYR")
  4 bytes skip


*/

void Loader::load(const path& path)
{  
  in = fopen(path.c_str(), "rb");

  //TODO: endianness
  static constexpr uint64_t ACHTUNG = 0x21474e5554484341;
  static constexpr uint32_t MAP = 0x2050414d;
  static constexpr uint32_t LAYR = 0x5259414c;

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

using object_id = uint16_t;

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

  assert(uncompressed.length == (width * height * sizeof(object_id)));



}

#include "game/Types.h"

#include <algorithm>
#include <vector>
#include <unordered_map>

namespace sutils
{
  using string_t = std::string;
  
  inline static bool startsWith(const string_t& string, const string_t& prefix) { return string.substr(0, prefix.length()) == prefix; }
  static inline std::string& ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
      return ch != ' ' && ch != '\t';
    }));
    return s;
  }
  static inline std::string& rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
      return ch != ' ' && ch != '\t';
    }).base(), s.end());
    return s;
  }
  static inline std::string& trim(std::string &s) {
    ltrim(s);
    rtrim(s);
    return s;
  }

  static inline std::string trimQuotes(const std::string& s)
  {
    return s.substr(1, s.length() - 2);
  }
}

class ValuesParser
{
private:
  baba::GameData& data;

  std::vector<std::string> lines;
  std::unordered_map<std::string, std::string> fields;
  size_t i = 0;
  size_t count = 0;

  enum class s { None, Finished, ListRoot, InsideObject };
  s state = s::None;

  void skipLine(size_t d = 1) { i += d; }
  std::pair<std::string, std::string> parseKeyValue(const std::string& v);

  void generateObject();

public:
  ValuesParser(baba::GameData& data) : data(data) { }

  void init();
  void parse();
};

std::pair<std::string, std::string> ValuesParser::parseKeyValue(const std::string& v)
{
  size_t idx = v.find(" =");
  if (idx == std::string::npos)
    return std::make_pair("", "");
  else
  {
    std::string key = v.substr(0, idx);
    std::string value = (v.length() >= idx + 3) ? v.substr(idx + 3) : "";
    if (!value.empty() && value.back() == ',') value.pop_back();
    return std::make_pair(key, value);
  }
}

void ValuesParser::generateObject()
{
  if (fields.find("name") != fields.end())
  {
    baba::ObjectSpec object;

    assert(fields.find("name") != fields.end());
    assert(fields.find("sprite_in_root") != fields.end());


    object.name = sutils::trimQuotes(fields["name"]);
    object.sprite = sutils::trimQuotes(fields["name"]);

    data.objects.push_back(object);
  }
}


void ValuesParser::init()
{
  const path valuesFile = DATA_FOLDER + "values.lua";
  
  std::ifstream file(valuesFile);

  if (file.is_open())
  {
    lines.clear();
    
    std::string line;
    while (getline(file, line))
      lines.push_back(line);

    file.close();

    count = lines.size();
    i = 0;
  }
}

void ValuesParser::parse()
{
  for (i = 0; i < lines.size(); ++i)
  {
    auto& line = lines[i];
    sutils::trim(line);

    /* skip comments */
    if (sutils::startsWith(line, "--") || line.empty())
      continue;

    switch (state)
    {
      case s::None:
      {
        if (line == "tileslist =")
        {
          state = s::ListRoot;
          skipLine();
        }
        break;
      }
      case s::ListRoot:
      {
        if (line == "}")
          state = s::Finished;
        else
        {
          auto objectName = parseKeyValue(line);
          assert(!objectName.first.empty() && objectName.second.empty());
          LOGD("Parsing object %s", objectName.first.c_str());
          skipLine();
          state = s::InsideObject;
          fields.clear();
        }

        break;
      }
      case s::InsideObject:
      {
        if (line == "},")
        {
          state = s::ListRoot;
          generateObject();
        }
        else
        {
          auto pair = parseKeyValue(line);
          fields.emplace(pair);
          assert(!pair.first.empty() && !pair.second.empty());
          LOGD("  Field %s = %s", pair.first.c_str(), pair.second.c_str());
        }

        break;
      }
    }
  }
}

void Loader::loadGameData()
{
  baba::GameData data;
  ValuesParser parser(data);
  parser.init();
  parser.parse();
}