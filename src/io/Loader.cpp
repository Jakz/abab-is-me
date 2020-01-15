#include "Loader.h"

#include "Zip.h"

#include "game/Types.h"
#include "game/Level.h"

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>

#if !_WIN32
namespace std
{
  int stoi(const std::string& str) { return atoi(str.c_str()); }
}
#endif

using namespace io;
using namespace baba;

Loader::Loader(baba::GameData& data) : data(data)
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

baba::Level* Loader::load(const path& p)
{
  LOGD("Loading level %s", p.c_str());
  path fullPath = DATA_FOLDER + R"(Worlds/baba/)" + p;
  in = fopen(fullPath.c_str(), "rb");
  assert(in);

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

  baba::Level* level = nullptr;

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
        level = readLayer(version, level);
      break;
    }
    else
      assert(false);

  }

  fclose(in);
  return level;
}

using object_id = uint16_t;

baba::Level* Loader::readLayer(uint16_t version, baba::Level* level)
{
  uint32_t width = read<uint32_t>(in);
  uint32_t height = read<uint32_t>(in);

  if (!level)
    level = new baba::Level(data, width, height);

  std::vector<Object> objects;

  LOGD("  reading a layer of %dx%d cells", width, height);

  if (version >= 258) skip(4, in);
  skip(25, in);
  if (version == 260) skip(2, in);
  else if (version == 261) skip(3, in);

  uint8_t dataBlocks = read<uint8_t>(in);
  assert(dataBlocks == 1 || dataBlocks == 2);

  /* MAIN */
  skip(4, in);

  {
    uint32_t compressedSize = read<uint32_t>(in);
    Zip::byte* buffer = (Zip::byte*)std::calloc(compressedSize, 1);
    fread(buffer, 1, compressedSize, in);
    auto uncompressed = Zip::uncompress(buffer, compressedSize);
    free(buffer);

    assert(uncompressed.length == (width * height * sizeof(object_id)));


    /* layer data is made by 2 bytes object indices */
    const int16_t* identifiers = reinterpret_cast<const int16_t*>(uncompressed.data);
    for (uint32_t i = 0; i < width*height; ++i)
    {
      int16_t id = identifiers[i];

      auto it = data.objectsByID.find(id);

      if (it != data.objectsByID.end())
        objects.push_back({ it->second });
      else
        objects.push_back({ nullptr }); //TODO: EMPTY object
    }
  }

  if (dataBlocks == 2)
  {
    skip(9, in);
    uint32_t compressedSize = read<uint32_t>(in);
    Zip::byte* buffer = (Zip::byte*)std::calloc(compressedSize, 1);
    fread(buffer, 1, compressedSize, in);
    auto uncompressed = Zip::uncompress(buffer, compressedSize);
    free(buffer);

    assert(uncompressed.length == width * height);

    for (uint32_t i = 0; i < width*height; ++i)
    {
      objects[i].variant = uncompressed.data[i];

      /*if (objects[i].spec && objects[i].spec->tiling == ObjectSpec::Tiling::Character)
      {
        printf("Variant %s: %d\n", objects[i].spec->name.c_str(), objects[i].variant);
      }*/
    }
  }

  for (uint32_t i = 0; i < width*height; ++i)
    if (objects[i].spec)
      level->get(i)->add(objects[i]);

  return level;
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
  std::pair<int32_t, int32_t> parseCoordinate(const std::string& v);

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

std::pair<int32_t, int32_t> ValuesParser::parseCoordinate(const std::string& v)
{
  assert(v.front() == '{' && v.back() == '}');
  auto idx = v.find_first_of(", ");
  assert(idx != std::string::npos);
  int32_t f = std::stoi(v.substr(1, idx - 1));
  int32_t s = std::stoi(v.substr(idx + 2));
  return std::make_pair(f, s);
}

void ValuesParser::generateObject()
{
  if (fields.find("name") != fields.end())
  {
    baba::ObjectSpec object;

    assert(fields.find("name") != fields.end());
    assert(fields.find("sprite") != fields.end());
    assert(fields.find("sprite_in_root") != fields.end());
    assert(fields.find("unittype") != fields.end());

    assert(fields.find("tile") != fields.end());
    assert(fields.find("colour") != fields.end());

    object.name = sutils::trimQuotes(fields["name"]);
    object.sprite = sutils::trimQuotes(fields["sprite"]);
    object.layer = std::stoi(fields["layer"]);

    auto tile = parseCoordinate(fields["tile"]);
    object.id = tile.first | (tile.second << 8);

    auto color = parseCoordinate(fields["colour"]);
    object.color = { color.first, color.second };

    auto grid = parseCoordinate(fields["grid"]);
    object.grid = { grid.first, grid.second };

    {
      auto type = sutils::trimQuotes(fields["unittype"]);

      if (type == "object")
        object.isText = false;
      else if (type == "text")
        object.isText = true;
      else
        assert(false);
    }

    if (object.isText)
    {
      auto active = parseCoordinate(fields["active"]);
      object.active = { active.first, active.second };
    }
    else
    {
      assert(fields.find("active") == fields.end());
      object.active = object.color;
    }

    {
      auto spriteInRoot = fields["sprite_in_root"];

      if (spriteInRoot == "true")
        object.spriteInRoot = true;
      else if (spriteInRoot == "false")
        object.spriteInRoot = false;
      else
        assert(false);

    }

    {
      int type = std::stoi(fields["type"]);
      using T = baba::ObjectSpec::Type;

      switch (type) {
      case 0: object.type = T::Noun; break;
      case 1: object.type = T::Verb; break;
      case 2: object.type = T::Property; break;
      case 3: object.type = T::Adjective; break;
      case 4: object.type = T::Negative; break;
      case 6: object.type = T::Conjunction; break;
      case 7: object.type = T::Preposition; break;
      default: assert(false);
      }
    }

    assert(object.isText || object.type == baba::ObjectSpec::Type::Noun);

    {
      int tiling = std::stoi(fields["tiling"]);

      switch (tiling) {
      case -1: object.tiling = baba::ObjectSpec::Tiling::None; break;
      case 0: object.tiling = baba::ObjectSpec::Tiling::Directions; break;
      case 1: object.tiling = baba::ObjectSpec::Tiling::Tiled; break;
      case 2: object.tiling = baba::ObjectSpec::Tiling::Character; break;
      case 3: object.tiling = baba::ObjectSpec::Tiling::Belt; break;
      default: assert(false);
      }
    }

    data.objects.push_back(object);
  }
  else
  {
    assert(fields["entry_name"] == "edge");

    baba::ObjectSpec edge;
    auto color = parseCoordinate(fields["colour"]);
    edge.color = { color.first, color.second };

    auto tile = parseCoordinate(fields["tile"]);
    edge.id = tile.first | (tile.second << 8);

    edge.name = "edge";

    data.objects.push_back(edge);
  }
}


void ValuesParser::init()
{
  LOGDD("Loading game data from values.lua..");
  const path valuesFile = DATA_FOLDER + "values.lua";

  std::ifstream file(valuesFile);

  if (file.is_open())
  {
    lines.clear();

    std::string line;
    while (getline(file, line))
    {
      if (!line.empty() && line.back() == '\r') line.pop_back();
      lines.push_back(line);
    }

    file.close();

    count = lines.size();
    i = 0;
  }
  else
  {
    LOGDD("Unable to load values.lua from data folder, aborting.");
    abort();
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
          //LOGD("Parsing object %s", objectName.first.c_str());
          skipLine();
          state = s::InsideObject;
          fields.clear();
          fields["entry_name"] = objectName.first;
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
          //LOGD("  Field %s = %s", pair.first.c_str(), pair.second.c_str());
        }

        break;
      }
    }
  }

  LOGD("  loaded %zu objects.", data.objects.size());
}

void Loader::loadGameData()
{
  ValuesParser parser(data);
  parser.init();
  parser.parse();
  data.finalize();
}
