/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <runtime/ext/ext_zlib.h>
#include <runtime/base/file/zip_file.h>
#include <util/compression.h>
#include <util/logger.h>
#ifdef HAVE_SNAPPY
#include <snappy.h>
#endif
#include <lz4.h>
#include <lz4hc.h>

#define PHP_ZLIB_MODIFIER 1000

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(zlib);
///////////////////////////////////////////////////////////////////////////////
// zlib functions

static Variant gzcompress(const char *data, int len, int level /* = -1 */) {
  if (level < -1 || level > 9) {
    throw_invalid_argument("level: %d", level);
    return false;
  }
  unsigned long l2 = len + (len / PHP_ZLIB_MODIFIER) + 15;
  String str(l2, ReserveString);
  char *s2 = str.mutableSlice().ptr;

  int status;
  if (level >= 0) {
    status = compress2((Bytef*)s2, &l2, (const Bytef*)data, len, level);
  } else {
    status = compress((Bytef*)s2, &l2, (const Bytef*)data, len);
  }

  if (status == Z_OK) {
    return str.shrink(l2);
  }

  Logger::Warning("%s", zError(status));
  return false;
}

static Variant gzuncompress(const char *data, int len, int limit /* = 0 */) {
  if (limit < 0) {
    Logger::Warning("length (%ld) must be greater or equal zero", limit);
    return false;
  }

  unsigned long plength = limit;
  unsigned long length;
  unsigned int factor = 4, maxfactor = 16;
  String str(std::max(plength, (unsigned long)StringData::MaxSmallSize),
             ReserveString);
  int status;
  do {
    length = plength ? plength : (unsigned long)len * (1 << factor++);
    if (length > StringData::MaxSize) {
      return false;
    }
    char* s2 = str.reserve(length).ptr;
    status = uncompress((Bytef*)s2, &length, (const Bytef*)data, len);
  } while ((status == Z_BUF_ERROR) && (!plength) && (factor < maxfactor));

  if (status == Z_OK) {
    return str.shrink(length);
  }
  Logger::Warning("%s", zError(status));
  return false;
}

///////////////////////////////////////////////////////////////////////////////

static Variant gzdeflate(const char *data, int len, int level /* = -1 */) {
  if (level < -1 || level > 9) {
    throw_invalid_argument("level: %d", level);
    return false;
  }
  z_stream stream;
  stream.data_type = Z_ASCII;
  stream.zalloc = (alloc_func) Z_NULL;
  stream.zfree  = (free_func) Z_NULL;
  stream.opaque = (voidpf) Z_NULL;

  stream.next_in = (Bytef *)data;
  stream.avail_in = len;

  stream.avail_out = len + (len / PHP_ZLIB_MODIFIER) + 15 + 1; // room for \0

  String str(stream.avail_out, ReserveString);
  char* s2 = str.mutableSlice().ptr;

  stream.next_out = (Bytef*)s2;

  /* init with -MAX_WBITS disables the zlib internal headers */
  int status = deflateInit2(&stream, level, Z_DEFLATED, -MAX_WBITS,
                            MAX_MEM_LEVEL, 0);
  if (status == Z_OK) {
    status = deflate(&stream, Z_FINISH);
    if (status != Z_STREAM_END) {
      deflateEnd(&stream);
      if (status == Z_OK) {
        status = Z_BUF_ERROR;
      }
    } else {
      status = deflateEnd(&stream);
    }
  }

  if (status == Z_OK) {
    /* resize to buffer to the "right" size */
    return str.shrink(stream.total_out);
  }
  Logger::Warning("%s", zError(status));
  return false;
}

static Variant gzinflate(const char *data, int len, int limit /* = 0 */) {
  if (len == 0) {
    return false;
  }

  if (limit < 0) {
    Logger::Warning("length (%ld) must be greater or equal zero", limit);
    return false;
  }
  unsigned long plength = limit;

  z_stream stream;
  stream.zalloc = (alloc_func) Z_NULL;
  stream.zfree = (free_func) Z_NULL;

  unsigned long length;
  int status;
  unsigned int factor = 4, maxfactor = 16;
  String str(std::max(plength, (unsigned long)StringData::MaxSmallSize),
             ReserveString);
  do {
    length = plength ? plength : (unsigned long)len * (1 << factor++);
    if (length > StringData::MaxSize) {
      return false;
    }
    char* s2 = str.reserve(length).ptr;

    stream.next_in = (Bytef *)data;
    stream.avail_in = (uInt)len + 1; /* there is room for \0 */

    stream.next_out = (Bytef*)s2;
    stream.avail_out = (uInt)length;

    /* init with -MAX_WBITS disables the zlib internal headers */
    status = inflateInit2(&stream, -MAX_WBITS);
    if (status == Z_OK) {
      status = inflate(&stream, Z_FINISH);
      if (status != Z_STREAM_END) {
        inflateEnd(&stream);
        if (status == Z_OK) {
          status = Z_BUF_ERROR;
        }
      } else {
        status = inflateEnd(&stream);
      }
    }
  } while ((status == Z_BUF_ERROR) && (!plength) && (factor < maxfactor));

  if (status == Z_OK) {
    return str.shrink(stream.total_out);
  }
  Logger::Warning("%s", zError(status));
  return false;
}

///////////////////////////////////////////////////////////////////////////////

Variant f_readgzfile(CStrRef filename, bool use_include_path /* = false */) {
  Object stream = f_gzopen(filename, "rb", use_include_path);
  if (stream.isNull()) {
    return false;
  }
  return f_gzpassthru(stream);
}

Variant f_gzfile(CStrRef filename, bool use_include_path /* = false */) {
  Object stream = f_gzopen(filename, "rb", use_include_path);
  if (stream.isNull()) {
    return false;
  }

  Array ret;
  Variant line;
  while (!same(line = f_gzgets(stream), false)) {
    ret.append(line);
  }
  return ret;
}

Variant f_gzcompress(CStrRef data, int level /* = -1 */) {
  return gzcompress(data.data(), data.size(), level);
}

Variant f_gzuncompress(CStrRef data, int limit /* = 0 */) {
  return gzuncompress(data.data(), data.size(), limit);
}

Variant f_gzdeflate(CStrRef data, int level /* = -1 */) {
  return gzdeflate(data.data(), data.size(), level);
}

Variant f_gzinflate(CStrRef data, int limit /* = 0 */) {
  return gzinflate(data.data(), data.size(), limit);
}

Variant f_gzencode(CStrRef data, int level /* = -1 */,
                   int encoding_mode /* = k_FORCE_GZIP */) {
  int len = data.size();
  char *ret = gzencode(data.data(), len, level, encoding_mode);
  if (ret == NULL) {
    return false;
  }
  return String(ret, len, AttachString);
}

Variant f_gzdecode(CStrRef data) {
  int len = data.size();
  char *ret = gzdecode(data.data(), len);
  if (ret == NULL) {
    return false;
  }
  return String(ret, len, AttachString);
}

///////////////////////////////////////////////////////////////////////////////
// stream functions

Object f_gzopen(CStrRef filename, CStrRef mode,
                bool use_include_path /* = false */) {
  File *file = NEWOBJ(ZipFile)();
  Object handle(file);
  bool ret = file->open(File::TranslatePath(filename), mode);
  if (!ret) {
    raise_warning("%s",Util::safe_strerror(errno).c_str());
    return NULL;
  }
  return handle;
}

///////////////////////////////////////////////////////////////////////////////
// QuickLZ functions

#ifdef HAVE_QUICKLZ

namespace QuickLZ1 {
#ifdef QLZ_COMPRESSION_LEVEL
#undef QLZ_COMPRESSION_LEVEL
#endif
#ifdef QLZ_STREAMING_BUFFER
#undef QLZ_STREAMING_BUFFER
#endif
#define QLZ_COMPRESSION_LEVEL 1
#define QLZ_STREAMING_BUFFER 0
#include "quicklz.inc"
}

namespace QuickLZ2 {
#ifdef QLZ_COMPRESSION_LEVEL
#undef QLZ_COMPRESSION_LEVEL
#endif
#ifdef QLZ_STREAMING_BUFFER
#undef QLZ_STREAMING_BUFFER
#endif
#define QLZ_COMPRESSION_LEVEL 2
#define QLZ_STREAMING_BUFFER 100000
#include "quicklz.inc"
}

namespace QuickLZ3 {
#ifdef QLZ_COMPRESSION_LEVEL
#undef QLZ_COMPRESSION_LEVEL
#endif
#ifdef QLZ_STREAMING_BUFFER
#undef QLZ_STREAMING_BUFFER
#endif
#define QLZ_COMPRESSION_LEVEL 3
#define QLZ_STREAMING_BUFFER 1000000
#include "quicklz.inc"
}

#endif // HAVE_QUICKLZ

Variant f_qlzcompress(CStrRef data, int level /* = 1 */) {
#ifndef HAVE_QUICKLZ
  throw NotSupportedException(__func__, "QuickLZ library cannot be found");
#else
  if (level < 1 || level > 3) {
    throw_invalid_argument("level: %d", level);
    return false;
  }

  String str(data.size() + 400, ReserveString);
  char* compressed = str.mutableSlice().ptr;
  size_t size;

  switch (level) {
    case 1: {
      QuickLZ1::qlz_state_compress state;
      memset(&state, 0, sizeof(state));
      size = QuickLZ1::qlz_compress(data.data(), compressed, data.size(),
                                    &state);
      break;
    }
    case 2: {
      QuickLZ2::qlz_state_compress state;
      memset(&state, 0, sizeof(state));
      size = QuickLZ2::qlz_compress(data.data(), compressed, data.size(),
                                    &state);
      break;
    }
    case 3:
      QuickLZ3::qlz_state_compress *state = new QuickLZ3::qlz_state_compress();
      memset(state, 0, sizeof(*state));
      size = QuickLZ3::qlz_compress(data.data(), compressed, data.size(),
                                    state);
      delete state;
      break;
  }

  ASSERT(size <= (size_t)data.size() + 400);
  return str.shrink(size);
#endif
}

Variant f_qlzuncompress(CStrRef data, int level /* = 1 */) {
#ifndef HAVE_QUICKLZ
  throw NotSupportedException(__func__, "QuickLZ library cannot be found");
#else
  if (level < 1 || level > 3) {
    throw_invalid_argument("level: %d", level);
    return false;
  }

  if (data.size() < 9) {
    raise_notice("passing invalid data to qlzuncompress()");
    return false;
  }

  size_t size = QuickLZ1::qlz_size_decompressed(data.data());
  if ((int64)size < 0 ||
      (RuntimeOption::SerializationSizeLimit > 0 &&
       (int64)size > RuntimeOption::SerializationSizeLimit)) {
    raise_notice("invalid size in compressed header: %lld", (int64)size);
    return false;
  }

  String s = String(size, ReserveString);
  char *decompressed = s.mutableSlice().ptr;
  size_t dsize;

  switch (level) {
    case 1: {
      QuickLZ1::qlz_state_decompress state;
      memset(&state, 0, sizeof(state));
      dsize = QuickLZ1::qlz_decompress(data.data(), decompressed, &state);
      break;
    }
    case 2: {
      QuickLZ2::qlz_state_decompress state;
      memset(&state, 0, sizeof(state));
      dsize = QuickLZ2::qlz_decompress(data.data(), decompressed, &state);
      break;
    }
    case 3: {
      QuickLZ3::qlz_state_decompress *state =
        new QuickLZ3::qlz_state_decompress();
      memset(state, 0, sizeof(*state));
      dsize = QuickLZ3::qlz_decompress(data.data(), decompressed, state);
      delete state;
      break;
    }
  }

  ASSERT(dsize == size);
  return s.setSize(dsize);
#endif
}

Variant f_sncompress(CStrRef data) {
#ifndef HAVE_SNAPPY
  throw NotSupportedException(__func__, "Snappy library cannot be found");
#else
  size_t size;
  char *compressed =
    (char *)malloc(snappy::MaxCompressedLength(data.size()) + 1);

  snappy::RawCompress(data.data(), data.size(), compressed, &size);
  compressed = (char *)realloc(compressed, size + 1);
  compressed[size] = '\0';
  return String(compressed, size, AttachString);
#endif
}

Variant f_snuncompress(CStrRef data) {
#ifndef HAVE_SNAPPY
  throw NotSupportedException(__func__, "Snappy library cannot be found");
#else
  size_t dsize;

  snappy::GetUncompressedLength(data.data(), data.size(), &dsize);
  String s = String(dsize, ReserveString);
  char *uncompressed = s.mutableSlice().ptr;

  if (!snappy::RawUncompress(data.data(), data.size(), uncompressed)) {
    return false;
  }
  return s.setSize(dsize);
#endif
}

#define NZLIB_MAGIC 0x6e7a6c69 /* nzli */
/* The new compression format stores a magic number and the size
   of the uncompressed object.  The magic number is stored to make sure
   bad values do not cause us to allocate bogus or extremely large amounts
   of memory when encountering an object with the new format. */
typedef struct nzlib_format_s {
    uint32_t magic;
    uint32_t uncompressed_sz;
    Bytef buf[0];
} nzlib_format_t;

Variant f_nzcompress(CStrRef uncompressed) {
  size_t len = compressBound(uncompressed.size());
  String str(sizeof(nzlib_format_t) + len, ReserveString);
  nzlib_format_t* format = (nzlib_format_t*)str.mutableSlice().ptr;

  format->magic = htonl(NZLIB_MAGIC);
  format->uncompressed_sz = htonl(uncompressed.size());

  int rc = compress(format->buf, &len, (uint8_t*)uncompressed.data(),
                    uncompressed.size());
  if (rc == Z_OK) {
    return str.shrink(len + sizeof(*format));
  }
  return false;
}

Variant f_nzuncompress(CStrRef compressed) {
  if (compressed.size() < (ssize_t)sizeof(nzlib_format_t)) {
    return false;
  }

  nzlib_format_t* format = (nzlib_format_t*)compressed.data();
  if (ntohl(format->magic) != NZLIB_MAGIC) {
    return false;
  }

  size_t len = ntohl(format->uncompressed_sz);
  if (len == 0) {
    return empty_string;
  }

  String str(len, ReserveString);
  char* uncompressed = str.mutableSlice().ptr;
  int rc = uncompress((Bytef*)uncompressed, &len, format->buf,
                      compressed.size() - sizeof(*format));
  if (rc != Z_OK) {
    return false;
  }

  if (format->uncompressed_sz == 0) {
    return str.shrink(len);
  }
  return str.setSize(len);
}

// Varint helper functions for lz4
int VarintSize(int val) {
  int s = 1;
  while (val >= 128) {
    ++s;
    val >>= 7;
  }
  return s;
}

void VarintEncode(int val, char** dest) {
  char* p = *dest;
  while (val >= 128) {
    *p++ = 0x80 | (static_cast<char>(val) & 0x7f);
    val >>= 7;
  }
  *p++ = static_cast<char>(val);
  *dest = p;
}

int VarintDecode(const char** src, int max_size) {
  const char* p = *src;
  int val = 0;
  int shift = 0;
  while (*p & 0x80) {
    if (max_size <= 1) { return -1; }
    --max_size;
    val |= static_cast<int>(*p++ & 0x7f) << shift;
    shift += 7;
  }
  val |= static_cast<int>(*p++) << shift;
  *src = p;
  return val;
}

Variant f_lz4compress(CStrRef uncompressed) {
  int bufsize = LZ4_compressBound(uncompressed.size());
  if (bufsize < 0) {
    return false;
  }
  int headerSize = VarintSize(uncompressed.size());
  bufsize += headerSize;  // for the header
  String s = String(bufsize, ReserveString);
  char *compressed = s.mutableSlice().ptr;

  VarintEncode(uncompressed.size(), &compressed);  // write the header

  int csize = LZ4_compress(uncompressed.data(), compressed,
      uncompressed.size());
  if (csize < 0) {
    return false;
  }
  bufsize = csize + headerSize;
  s.shrink(bufsize);
  return s.setSize(bufsize);
}

Variant f_lz4hccompress(CStrRef uncompressed) {
  int bufsize = LZ4_compressBound(uncompressed.size());
  if (bufsize < 0) {
    return false;
  }
  int headerSize = VarintSize(uncompressed.size());
  bufsize += headerSize;  // for the header
  String s = String(bufsize, ReserveString);
  char *compressed = s.mutableSlice().ptr;

  VarintEncode(uncompressed.size(), &compressed);  // write the header

  int csize = LZ4_compressHC(uncompressed.data(),
      compressed, uncompressed.size());
  if (csize < 0) {
    return false;
  }
  bufsize = csize + headerSize;
  return s.shrink(bufsize);
}

Variant f_lz4uncompress(CStrRef compressed) {
  const char* compressed_ptr = compressed.data();
  int dsize = VarintDecode(&compressed_ptr, compressed.size());
  if (dsize < 0) {
    return false;
  }

  String s = String(dsize, ReserveString);
  char *uncompressed = s.mutableSlice().ptr;
  int ret = LZ4_uncompress(compressed_ptr, uncompressed, dsize);

  if (ret <= 0) {
    return false;
  }
  return s.setSize(dsize);
}

///////////////////////////////////////////////////////////////////////////////
}
