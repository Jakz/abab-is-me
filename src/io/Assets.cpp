#include "Assets.h"

#include "Common.h"

#include <iostream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>
#include <filesystem>
#include <fstream>

#include "SDL.h"
#include "SDL_image.h"

using namespace std;

bool verbose = false;
auto out = std::fstream("output.txt", std::ios_base::out);//std::cout;

bool AssetLoader::decode(const std::string& assets, const std::string& outFolder)
{
  std::filesystem::create_directory(outFolder);

  file = fopen(assets.c_str(), "rb");

  if (!file)
  {
    cerr << "Error opening file: " << assets << endl;
    return false;
  }

  cacheOffsets();
  int data_index = 0;

  out << "Extracting images..." << endl;
  string image_folder = outFolder + "/images";
  std::filesystem::create_directory(image_folder.c_str());
  int image_index = 0;
  while (data_index < offsets.size() && tryExtractImage(offsets[data_index], image_folder, data_index))
  {
    data_index++;
    image_index++;

    //if (data_index == 20)
    //  return true;
  }

  out << "Extracting sounds..." << endl;
  string sound_folder = outFolder + "/sounds";
  std::filesystem::create_directory(sound_folder.c_str());
  int sound_index = 0;
  while (data_index < offsets.size() && tryExtractSound(offsets[data_index], sound_folder + "/" + to_string(data_index) + ".ogg"))
  {
    data_index++;
    sound_index++;
  }

  out << "Extracting shaders..." << endl;
  string shader_folder = outFolder + "/shaders";
  std::filesystem::create_directory(shader_folder.c_str());
  int shader_index = 0;
  while (data_index < offsets.size() && tryExtractShader(offsets[data_index], shader_folder + "/" + to_string(shader_index) + "_vert.hlsl", shader_folder + "/" + to_string(shader_index) + "_frag.hlsl"))
  {
    data_index++;
    shader_index++;
  }

  out << "Done!" << endl;

  fclose(file);

  return true;
}

void AssetLoader::cacheOffsets()
{
  file = fopen(_path.c_str(), "rb");
  
  uint32_t prev = 0;
  uint32_t offset;

  do
  {
    fread(&offset, sizeof(offset), 1, file);

    if (offset < prev)
    {
      fseek(file, -(int64_t)sizeof(offset), SEEK_CUR);
      break;
    }

    offsets.push_back(offset);
    prev = offset;

  } while (true);

  LOGD("Cached %zu asset offsets", offsets.size());

  fclose(file);
  file = nullptr;
}

std::vector<uint8_t> AssetLoader::loadSound(uint32_t index)
{
  if (index < offsets.size())
  {
    auto offset = offsets[index];
    file = fopen(_path.c_str(), "rb");

    int32_t length;

    fseek(file, offset, SEEK_SET);
    fseek(file, 4, SEEK_CUR);
    fread(&length, sizeof(int32_t), 1, file);

    /* read and write data */
    std::vector<uint8_t> buffer(length);

    fread(buffer.data(), 1, length, file);

    fclose(file);

    return buffer;
  }
  
  return { };
}

bool AssetLoader::tryExtractImage(uint32_t offset, const std::string& folder, size_t index)
{
  return false;
}

Surface AssetLoader::loadImage(uint32_t index)
{
  if (index >= offsets.size())
    return nullptr;
  
  auto offset = offsets[index];
  
  uint16_t width, height;
  int32_t length;

  file = fopen(_path.c_str(), "rb");

  fseek(file, offset, SEEK_SET);
  fread(&width, sizeof(width), 1, file);
  fread(&height, sizeof(height), 1, file);
  fseek(file, 9, SEEK_CUR);
  fread(&length, sizeof(length), 1, file);

  /*if (length == 0)
    return true;
  else if (width == 0 || height == 0)
  {
    fseek(file, -17, SEEK_CUR);
    return false;
  }*/

  if (verbose)
    out << " Image " << index << " " << width << "x" << height << " at offset " << offset << " (length " << length << ")" << std::endl;

  std::vector<uint8_t> uncompressed;

  auto end = ftell(file) + length;

  while (ftell(file) < end)
  {
    int32_t blockSize = read<uint32_t>();
    int32_t blockStart = ftell(file);

    if (verbose)
      out << "blockSize: " << blockSize << " blockStart: " << blockStart << std::endl;

    std::vector<uint8_t> data;
    data.resize(blockSize);
    fread(&data[0], 1, blockSize, file);
    fseek(file, -blockSize, SEEK_CUR);

    /*for (size_t i = 0; i < (blockSize / 16) + 1; ++i)
    {
      size_t base = i * 16;
      size_t count = 0;

      while (base + count < blockSize && count < 16)
      {
        printf("%02X ", data[base + count]);
        ++count;
      }

      printf("\n");
    }*/

    while (true)
    {
      uint32_t startOffset = tell() - blockStart;


      uint8_t split = read<uint8_t>();

      if (verbose)
        out << "split 0x" << std::hex << (int)split << std::dec << std::endl;

      if (split >= 0x10)
      {
        uint32_t take = split >> 4;

        if (take == 0x0f)
        {
          while (true)
          {
            uint8_t b = read<uint8_t>();
            take += b;

            if (b != 0xff)
              break;
          }
        }

        /* read take bytes */
        size_t currentSize = uncompressed.size();
        uncompressed.resize(currentSize + take);
        fread(&uncompressed[currentSize], 1, take, file);

        if (verbose)
          out << startOffset << ": taking " << (int)take << " bytes (" << tell() - blockStart << ")" << std::endl;        
      }

      if (tell() >= blockStart + blockSize)
        break;

      uint32_t repeatOffset = read<uint16_t>();
      //cout << "repeatOffset: " << repeatOffset << endl;
      int32_t repeatIndex = uncompressed.size() - repeatOffset;
      uint32_t repeat = split & 0x0f;

      if (repeat == 0x0f)
      {
        while (true)
        {
          uint8_t b = read<uint8_t>();
          repeat += b;

          if (b != 0xff)
            break;
        }
      }

      if (verbose)
        out << startOffset << ": repeating " << (repeat + 4) << " bytes from " << repeatIndex << endl;

      for (size_t i = 0; i < repeat + 4; ++i)
        uncompressed.push_back(uncompressed[repeatIndex + i]);

      if (tell() >= blockStart + blockSize)
        break;
    }
  }

  /*std::string outFolder = folder + "/" + std::to_string(width) + "x" + std::to_string(height);
  std::filesystem::create_directory(outFolder);
  std::string outPath = outFolder + "/" + std::to_string(index) + ".png";*/

  SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
  memcpy(surface->pixels, &uncompressed[0], uncompressed.size());
  /*IMG_SavePNG(surface, outPath.c_str());
  SDL_FreeSurface(surface);*/
  
  fclose(file);
  file = nullptr;


  return Surface(surface);
}

bool AssetLoader::tryExtractSound(uint32_t offset, const string& outPath)
{
  uint32_t length;
  char header[4];

  fseek(file, offset, SEEK_SET);
  fseek(file, 4, SEEK_CUR);
  fread(&length, sizeof(int32_t), 1, file);
  fread(header, 1, 4, file);

  /* not an ogg file, rewind and return */
  if (memcmp(header, "OggS", 4) != 0)
  {
    fseek(file, -8, SEEK_CUR);
    return false;
  }

  /* rewind header */
  fseek(file, -4, SEEK_CUR);

  //std::cout << " Sound " << outPath << ": " << length << " bytes" << std::endl;

  /* read and write data */
  std::byte* buffer = new std::byte[length];
  fread(buffer, 1, length, file);

  FILE* out = fopen(outPath.c_str(), "wb+");
  fwrite(buffer, 1, length, out);
  fclose(out);

  return true;
}

bool AssetLoader::tryExtractShader(uint32_t offset, const string& vert_path, const string& frag_path)
{
  uint32_t length;
  char header[8];

  fseek(file, offset, SEEK_SET);
  fread(&length, sizeof(int32_t), 1, file);
  fread(header, 1, 4, file);

  /* not an shader file, rewind and return */
  if (memcmp(header, "#version", 8) != 0)
  {
    fseek(file, -12, SEEK_CUR);
    return false;
  }

  std::cout << " Shader " << vert_path << " " << frag_path << ": " << length << " bytes" << std::endl;

  fseek(file, length, SEEK_CUR);
  fread(&length, sizeof(int32_t), 1, file);
  fseek(file, length, SEEK_CUR);

  return true;
}