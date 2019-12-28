#pragma once

#include <cstdlib>
#include <cstdint>

class Zip
{
public:
  using byte = uint8_t;
  using u32 = uint32_t;

  struct unique_cptr
  {
    byte* data;
    size_t length;

    unique_cptr(byte* data, size_t length) : data(data), length(length) { }
    ~unique_cptr() { std::free(data); }
  };

private:
  static int inflateInternal(const byte* dataIn, size_t length, byte** dataOut, size_t* outLength, size_t outLenghtHint);
    
public:
  static unique_cptr compress(byte *data, u32 length, u32& flength, u32 bufferSize = 2048);
    
  static void uncompress(byte *data, u32 length, byte* dest, u32 destLength);
  static unique_cptr uncompress(const byte* data, size_t length);
};