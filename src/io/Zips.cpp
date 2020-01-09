#include "Zip.h"

#include <zlib.h>

#include <unordered_map>

Zip::unique_cptr Zip::compress(byte *data, u32 length, u32& flength, u32 bufferSize)
{
  if (length == 0)
  {
    flength = length;
    return unique_cptr(data, 0);
  }
  
  z_stream strm;
  
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.total_out = 0;
  strm.next_in = data;
  strm.avail_in = length;
  
  // Compresssion Levels:
  //   Z_NO_COMPRESSION
  //   Z_BEST_SPEED
  //   Z_BEST_COMPRESSION
  //   Z_DEFAULT_COMPRESSION
  
  if (deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, (15+16), 8, Z_DEFAULT_STRATEGY) != Z_OK) return unique_cptr(nullptr, 0);
  
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
    strm.avail_out = static_cast<uInt>(tlength - strm.total_out);
    
    deflate(&strm, Z_FINISH);
    
  } while (strm.avail_out == 0);
  
  deflateEnd(&strm);
  
  //JJ_DEBUG("COMPRESSION: %d -> %d (%2.4f)",length, strm.total_out, strm.total_out/(float)length)
  
  flength = static_cast<u32>(strm.total_out);
  return unique_cptr(compressed, flength);
}

void Zip::uncompress(byte *data, u32 length, byte *dest, u32 tlength)
{
  if (length == 0)
    return;
  
  bool done = false;
  
  z_stream strm;
  strm.next_in = data;
  strm.avail_in = length;
  strm.total_out = 0;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  
  if (inflateInit2(&strm, (15+32)) != Z_OK)
    return;
  
  while (!done)
  {
    strm.next_out = dest + strm.total_out;
    strm.avail_out = static_cast<uInt>(tlength - strm.total_out);
    
    // Inflate another chunk.
    int status = inflate (&strm, Z_SYNC_FLUSH);
    if (status == Z_STREAM_END)
      done = true;
    else if (status != Z_OK)
      break;
  }
  
  if (inflateEnd (&strm) != Z_OK)
    return;
  
  return;
}

int Zip::inflateInternal(const byte* dataIn, size_t length, byte** dataOut, size_t* outLength, size_t outLenghtHint)
{
  static constexpr size_t FACTOR = 2;
  int err = Z_OK;
  
  size_t bufferSize = outLenghtHint ;
  byte *out = static_cast<unsigned char*>(malloc(bufferSize));
  *dataOut = out;
  
  z_stream d_stream; /* decompression stream */
  d_stream.zalloc = (alloc_func)0;
  d_stream.zfree = (free_func)0;
  d_stream.opaque = (voidpf)0;
  
  d_stream.next_in  = const_cast<byte*>(dataIn);
  d_stream.avail_in = static_cast<unsigned int>(length);
  d_stream.total_in = 0;
  d_stream.next_out = out;
  d_stream.avail_out = static_cast<unsigned int>(bufferSize);
  d_stream.total_out = 0;

  if (!out)
    return Z_BUF_ERROR;

  if ((err = inflateInit2(&d_stream, MAX_WBITS)) != Z_OK)
  {
    free(out);
    return err;
  }
  
  for (;;)
  {
    err = inflate(&d_stream, Z_NO_FLUSH);
    
    if (err == Z_STREAM_END)
      break;
    
    switch (err)
    {
      case Z_NEED_DICT:
        printf("Need Dict: %s\n", d_stream.msg);
        inflateEnd(&d_stream);
        return err;
        break;
      case Z_DATA_ERROR:
        printf("Data Error: %s\n", d_stream.msg);
        inflateEnd(&d_stream);
        return err;
        break;
      case Z_MEM_ERROR:
        inflateEnd(&d_stream);
        return err;
    }
    
    if (err != Z_STREAM_END)
    {
      auto newOut = static_cast<byte*>(realloc(out, bufferSize * FACTOR));
      
      if (!newOut)
      {
        free(out);
        inflateEnd(&d_stream);
        return Z_MEM_ERROR;
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
  err = inflateEnd(&d_stream);
  return err;
}

Zip::unique_cptr Zip::uncompress(const byte* data, size_t length)
{
  byte* dataOut;
  size_t dataOutLength;

  int result = inflateInternal(data, length, &dataOut, &dataOutLength, 64 * 1024);
  
  if (result != Z_OK || !dataOut)
  {
    switch (result)
    {
      case Z_MEM_ERROR: printf("cocos2d: ZipUtils: Out of memory while decompressing map data!" "\n"); break;
      case Z_VERSION_ERROR: printf("cocos2d: ZipUtils: Incompatible zlib version!" "\n"); break;
      case Z_DATA_ERROR: printf("cocos2d: ZipUtils: Incorrect zlib compressed data!" "\n"); break;
      default: printf("cocos2d: ZipUtils: Unknown error while decompressing map data!" "\n");
    }
    
    free(dataOut);
    dataOut = nullptr;
  }
  
  return unique_cptr(dataOut, dataOutLength);
}
