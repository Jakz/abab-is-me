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
#else
#include <filesystem>
#include <regex>
#endif

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

  static inline std::vector<std::string> readFileToLines(const std::string& path)
  {
    std::vector<std::string> lines;
    std::ifstream file(path);

    if (file.is_open())
    {
      std::string line;
      while (getline(file, line))
      {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        lines.push_back(line);
      }

      file.close();
    }

    return lines;
  }

  static inline std::vector<path> contentsOfFolder(const path& base, bool recursive, std::function<bool(path)> filter)
  {
    std::vector<path> files;

    for (const auto& entry : std::filesystem::directory_iterator(base))
    {
      if (entry.is_regular_file() && filter(entry.path().filename().string()))
        files.push_back(entry.path().filename().string());
    }

    return files;
    
    /*DIR *d;
    struct dirent *dir;
    d = opendir(base.c_str());

    if (d)
    {
      while ((dir = readdir(d)) != NULL)
      {
        path name = path(dir->d_name);

        if (name == "." || name == ".." || name == ".DS_Store" || excludePredicate(name))
          continue;
        else if (dir->d_type == DT_DIR && recursive)
        {
          auto rfiles = contentsOfFolder(base.append(name), recursive, excludePredicate);
          files.reserve(files.size() + rfiles.size());
          std::move(rfiles.begin(), rfiles.end(), std::back_inserter(files));
        }
        else if (dir->d_type == DT_REG)
          files.push_back(base.append(name));
      }

      closedir(d);
    }*/
  }

  static std::pair<std::string, std::string> parseKeyValue(const std::string& v, const std::string delim = "=")
  {
    size_t idx = v.find(delim);
    if (idx == std::string::npos)
      return std::make_pair("", "");
    else
    {
      std::string key = v.substr(0, idx);
      std::string value = (v.length() >= idx + 1) ? v.substr(idx + 1) : "";
      if (!value.empty() && value.back() == ',') value.pop_back();
      return std::make_pair(sutils::trim(key), sutils::trim(value));
    }
  }

  static std::pair<int32_t, std::string> parseNumberKeyValue(const std::string& v)
  {
    std::string number = "";

    size_t i = 0;

    while (v[i] >= '0' && v[i] <= '9')
    {
      number += v[i];
      ++i;
    }

    return std::make_pair(std::stoi(number), v.substr(i));
  }

  static point_t parseCoordinate(std::string v)
  {
    //TODO: optimizable
    
    if (v.front() == '{' && v.back() == '}')
      v = v.substr(1, v.size() - 2);
    
    auto idx = v.find_first_of(",");
    assert(idx != std::string::npos);

    auto fh = v.substr(0, idx), sh = v.substr(idx + 1);

    int32_t f = std::stoi(sutils::trim(fh));
    int32_t s = std::stoi(sutils::trim(sh));
    return { f, s };
  }

  baba::ObjectSpec::Tiling valueToTiling(int value)
  {
    switch (value) {
    case -1: return baba::ObjectSpec::Tiling::None;
    case 0: return baba::ObjectSpec::Tiling::Directions;
    case 1: return baba::ObjectSpec::Tiling::Tiled;
    case 2: return baba::ObjectSpec::Tiling::Character;
    case 3: return baba::ObjectSpec::Tiling::Belt;
    case 4: return baba::ObjectSpec::Tiling::Unknown;
    default: assert(false); return baba::ObjectSpec::Tiling::None;
    }
  }
}

using namespace io;
using namespace baba;

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

void Loader::loadLD(const path& path, baba::Level* level, bool headerOnly)
{
  enum class S { NONE, GENERAL, IMAGES, SPECIALS, ICONS, PATHS, TILES, LEVELS };
  S s = S::NONE;

  auto lines = sutils::readFileToLines(path);

  for (int i = 0; i < lines.size(); ++i)
  {
    const auto& l = lines[i];
    
    //TODO: ignore comments which start with --

    if (l[0] == '[')
    {
      if (l == "[general]")
        s = S::GENERAL;
      else if (l == "[tiles]")
        s = S::TILES;
      else if (l == "[specials]")
        s = S::SPECIALS;
      else if (l == "[images]")
        s = S::IMAGES;
      else if (l == "[icons]" || l == "[levels]" || l == "[paths]")
        s = S::NONE;
      else
        assert(false);
    }
    else if (s == S::GENERAL)
    {
      auto pair = sutils::parseKeyValue(l);
      
      if (pair.first == "name")
        level->_info.name = pair.second;
      else if (pair.first == "palette")
        level->_palette = pair.second;
      else if (pair.first == "subtitle")
        level->_info.subtitle = pair.second;
    }
    else if (!headerOnly)
    {
      switch (s)
      {
      case S::GENERAL: break;
      case S::IMAGES:
      {
        auto pair = sutils::parseKeyValue(l);

        if (pair.first == "total")
        {
          assert(level->_images.empty());
          level->_images.resize(std::stoi(pair.second));
        }
        else
        {
          assert(!level->_images.empty());
          auto index = std::stoi(pair.first);
          const auto& image = pair.second;
          level->_images[index] = image;
        }

        break;
      }
      case S::SPECIALS:
      {
        auto p = sutils::parseKeyValue(l);
        auto nv = sutils::parseNumberKeyValue(p.first);

        //LOGD("special %d %s = %s", nv.first, nv.second.c_str(), p.second.c_str());
        break;
      }
      case S::TILES:
      {
        auto p = sutils::parseKeyValue(l);

        if (sutils::startsWith(p.first, "object"))
        {
          auto f = sutils::parseKeyValue(p.first, "_");

          auto* spec = level->_data.objectsByKey[f.first];

          if (f.second == "image")
            spec->sprite = p.second;
          else if (f.second == "name")
            spec->name = p.second;
          else if (f.second == "colour")
            spec->color = sutils::parseCoordinate(p.second);
          else if (f.second == "activecolour")
            spec->color = sutils::parseCoordinate(p.second);
          else if (f.second == "tiling")
            spec->tiling = sutils::valueToTiling(std::stoi(p.second));
          else if (f.second == "root")
            spec->spriteInRoot = p.second == "1" ? true : false;
          break;
        }
      }
      default: { }
      }
    }
  }
}

baba::Level* Loader::load(const std::string& name, const GameData& baseData)
{
  LOGD("Loading level %s", name.c_str());
  Level* level = new Level(baseData);

  path ldPath = DATA_FOLDER + R"(Worlds/baba/)" + name + ".ld";
  loadLD(ldPath, level);

  path fullPath = DATA_FOLDER + R"(Worlds/baba/)" + name + ".l";
  in = fopen(fullPath.c_str(), "rb");
  
  if (!in)
    return nullptr;

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
        readLayer(version, level);
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

  if (level->width() == 0)
    level->resize(width, height);
  else
    assert(level->width() == width && level->height() == height);

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

      auto it = level->_data.objectsByID.find(id);

      if (it != level->_data.objectsByID.end())
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
      auto& object = objects[i];
      
      object.variant = uncompressed.data[i];

      if (object.spec && object.spec->tiling == ObjectSpec::Tiling::Character)
      {
        if (object.variant == 0)
          object.direction = D::RIGHT;
      }
    }
  }

  for (uint32_t i = 0; i < width*height; ++i)
    if (objects[i].spec)
      level->get(i)->add(objects[i]);

  return level;
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

  void generateObject();

public:
  ValuesParser(baba::GameData& data) : data(data) { }

  void init();
  void parse();
};

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
    object.key = fields["entry_name"];
    object.layer = std::stoi(fields["layer"]);

    auto tile = sutils::parseCoordinate(fields["tile"]);
    object.id = tile.x | (tile.y << 8);

    object.color = sutils::parseCoordinate(fields["colour"]);
    object.grid = sutils::parseCoordinate(fields["grid"]);

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
      object.active = sutils::parseCoordinate(fields["active"]);
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

    object.tiling = sutils::valueToTiling(std::stoi(fields["tiling"]));

    data.objects.push_back(object);
  }
  else
  {
    assert(fields["entry_name"] == "edge");

    baba::ObjectSpec edge;
    edge.color = sutils::parseCoordinate(fields["colour"]);

    auto tile = sutils::parseCoordinate(fields["tile"]);
    edge.id = tile.x | (tile.y << 8);

    edge.name = "edge";

    data.objects.push_back(edge);
  }
}


void ValuesParser::init()
{
  LOGDD("Loading game data from values.lua..");
  const path valuesFile = DATA_FOLDER + "values.lua";

  lines = sutils::readFileToLines(valuesFile);
  count = lines.size();
  i = 0;

  if (count == 0)
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
          auto objectName = sutils::parseKeyValue(line);
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
          auto pair = sutils::parseKeyValue(line);
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

GameData Loader::loadGameData()
{
  //std::regex pattern(R"(^[0-9]+level.l$)");
  //auto levels = sutils::contentsOfFolder(DATA_FOLDER + R"(Worlds/baba)", false, [&pattern](auto path) { return std::regex_search(path, pattern); });
  
  GameData data;
  ValuesParser parser(data);
  parser.init();
  parser.parse();
  data.finalize();
  return data;
  
  /*coord_t mw = -1, mh = -1, ii = 0;

  for (uint32_t i = 0; i <= 327; ++i)
  {
    auto path = std::to_string(i) + "level";
    auto level = load(path);

    if (level && (mw < level->width() || mh < level->height()))
    {
      mw = std::max(mw, level->width());
      mh = std::max(mh, level->height());
      ii = i;
    }
  }

  LOGD("Max size: %d x %d for level %d", mw, mh, ii);*/
  
}
