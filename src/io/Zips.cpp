#include "Zip.h"

#include "io/zip/miniz.h"

#include <cstdio>
#include <unordered_map>

Zip::unique_cptr Zip::compress(byte *data, u32 length, u32& flength, u32 bufferSize)
{
  if (length == 0)
  {
    flength = length;
    return unique_cptr(data, 0);
  }

  mz_stream strm;

  strm.zalloc = nullptr;
  strm.zfree = nullptr;
  strm.opaque = nullptr;
  strm.total_out = 0;
  strm.next_in = data;
  strm.avail_in = length;

  // Compresssion Levels:
  //   Z_NO_COMPRESSION
  //   Z_BEST_SPEED
  //   Z_BEST_COMPRESSION
  //   Z_DEFAULT_COMPRESSION

  if (mz_deflateInit2(&strm, MZ_DEFAULT_COMPRESSION, MZ_DEFLATED, (15+16), 8, MZ_DEFAULT_STRATEGY) != MZ_OK) return unique_cptr(nullptr, 0);

  byte *compressed = (byte*)std::calloc(bufferSize,sizeof(byte));
  u32 tlength = bufferSize;

  do
  {
    if (strm.total_out >= tlength)
    {
      tlength += bufferSize;
      compressed = (byte*)std::realloc(compressed, sizeof(byte)*tlength);
    }

    strm.next_out = compressed + strm.total_out;
    strm.avail_out = static_cast<uint32_t>(tlength - strm.total_out);

    mz_deflate(&strm, MZ_FINISH);

  } while (strm.avail_out == 0);

  mz_deflateEnd(&strm);

  //JJ_DEBUG("COMPRESSION: %d -> %d (%2.4f)",length, strm.total_out, strm.total_out/(float)length)

  flength = static_cast<u32>(strm.total_out);
  return unique_cptr(compressed, flength);
}

void Zip::uncompress(byte *data, u32 length, byte *dest, u32 tlength)
{
  if (length == 0)
    return;

  bool done = false;

  mz_stream strm;
  strm.next_in = data;
  strm.avail_in = length;
  strm.total_out = 0;
  strm.zalloc = nullptr;
  strm.zfree = nullptr;

  if (mz_inflateInit2(&strm, (15+32)) != MZ_OK)
    return;

  while (!done)
  {
    strm.next_out = dest + strm.total_out;
    strm.avail_out = static_cast<uint32_t>(tlength - strm.total_out);

    // Inflate another chunk.
    int status = mz_inflate(&strm, MZ_SYNC_FLUSH);
    if (status == MZ_STREAM_END)
      done = true;
    else if (status != MZ_OK)
      break;
  }

  if (mz_inflateEnd(&strm) != MZ_OK)
    return;

  return;
}

int Zip::inflateInternal(const byte* dataIn, size_t length, byte** dataOut, size_t* outLength, size_t outLenghtHint)
{
  static constexpr size_t FACTOR = 2;
  int err = MZ_OK;

  size_t bufferSize = outLenghtHint ;
  byte *out = static_cast<unsigned char*>(malloc(bufferSize));
  *dataOut = out;

  mz_stream d_stream; /* decompression stream */
  d_stream.zalloc = nullptr;
  d_stream.zfree = nullptr;
  d_stream.opaque = nullptr;

  d_stream.next_in  = const_cast<byte*>(dataIn);
  d_stream.avail_in = static_cast<unsigned int>(length);
  d_stream.total_in = 0;
  d_stream.next_out = out;
  d_stream.avail_out = static_cast<unsigned int>(bufferSize);
  d_stream.total_out = 0;

  if (!out)
    return MZ_BUF_ERROR;

  if ((err = mz_inflateInit2(&d_stream, 15)) != MZ_OK)
  {
    free(out);
    return err;
  }

  for (;;)
  {
    err = mz_inflate(&d_stream, MZ_NO_FLUSH);

    if (err == MZ_STREAM_END)
      break;

    switch (err)
    {
      case MZ_NEED_DICT:
        printf("Need Dict: %s\n", d_stream.msg);
        mz_inflateEnd(&d_stream);
        return err;
        break;
      case MZ_DATA_ERROR:
        printf("Data Error: %s\n", d_stream.msg);
        mz_inflateEnd(&d_stream);
        return err;
        break;
      case MZ_MEM_ERROR:
        mz_inflateEnd(&d_stream);
        return err;
    }

    if (err != MZ_STREAM_END)
    {
      auto newOut = static_cast<byte*>(realloc(out, bufferSize * FACTOR));

      if (!newOut)
      {
        free(out);
        mz_inflateEnd(&d_stream);
        return MZ_MEM_ERROR;
      }
      else
        out = newOut;

      d_stream.next_out = out + bufferSize;
      d_stream.avail_out = static_cast<unsigned int>(bufferSize);
      bufferSize *= FACTOR;
    }
  }

  *outLength = bufferSize - d_stream.avail_out;
  *dataOut = reinterpret_cast<byte*>(realloc(out, *outLength));
  err = mz_inflateEnd(&d_stream);
  return err;
}

Zip::unique_cptr Zip::uncompress(const byte* data, size_t length)
{
  byte* dataOut;
  size_t dataOutLength;

  int result = inflateInternal(data, length, &dataOut, &dataOutLength, 64 * 1024);

  if (result != MZ_OK || !dataOut)
  {
    switch (result)
    {
      case MZ_MEM_ERROR: printf("Zip: Out of memory while decompressing map data!" "\n"); break;
      case MZ_VERSION_ERROR: printf("Zip: Incompatible zlib version!" "\n"); break;
      case MZ_DATA_ERROR: printf("Zip: Incorrect zlib compressed data!" "\n"); break;
      default: printf("Zip: Unknown error while decompressing map data!" "\n");
    }

    free(dataOut);
    dataOut = nullptr;
  }

  return unique_cptr(dataOut, dataOutLength);
}
