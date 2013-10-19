/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/ext_zlib.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/mem-file.h"
#include "hphp/runtime/base/zip-file.h"
#include "hphp/runtime/base/stream-wrapper.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/file-stream-wrapper.h"
#include "hphp/util/compression.h"
#include "hphp/util/logger.h"
#include "folly/String.h"
#ifdef HAVE_SNAPPY
#include <snappy.h>
#endif
#include <lz4.h>
#include <lz4hc.h>
#include <memory>

#define PHP_ZLIB_MODIFIER 1000

using namespace HPHP;

namespace {

///////////////////////////////////////////////////////////////////////////////
// compress.zlib:// stream wrapper

static class ZlibStreamWrapper : public Stream::Wrapper {
 public:
  virtual File* open(const String& filename, const String& mode,
                     int options, CVarRef context) {
    String fname;
    static const char cz[] = "compress.zlib://";

    if (!strncmp(filename.data(), "zlib:", sizeof("zlib:") - 1)) {
      fname = filename.substr(sizeof("zlib:") - 1);
    } else if (!strncmp(filename.data(), cz, sizeof(cz) - 1)) {
      fname = filename.substr(sizeof(cz) - 1);
    } else {
      return NULL;
    }

    if (MemFile* file = FileStreamWrapper::openFromCache(fname, mode)) {
      file->unzip();
      return file;
    }

    std::unique_ptr<ZipFile> file(NEWOBJ(ZipFile)());
    bool ret = file->open(File::TranslatePath(fname), mode);
    if (!ret) {
      raise_warning("%s", file->getLastError().c_str());
      return NULL;
    }
    return file.release();
  }
} s_zlib_stream_wrapper;

///////////////////////////////////////////////////////////////////////////////
// Extension entry point

static class ZlibExtension : Extension {
 public:
  ZlibExtension() : Extension("zlib") {}
  virtual void moduleLoad(Hdf hdf) {
    s_zlib_stream_wrapper.registerAs("compress.zlib");
  }
} s_zlib_extension;

} // nil namespace

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// zlib functions

static Variant gzcompress(const char *data, int len, int level /* = -1 */) {
  if (level < -1 || level > 9) {
    throw_invalid_argument("level: %d", level);
    return false;
  }
  unsigned long l2 = len + (len / PHP_ZLIB_MODIFIER) + 15;
  String str(l2, ReserveString);
  char *s2 = str.bufferSlice().ptr;

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
    Logger::Warning("length (%d) must be greater or equal zero", limit);
    return false;
  }

  unsigned long plength = limit;
  unsigned long length;
  unsigned int factor = 4, maxfactor = 16;
  String str(std::max(plength, (unsigned long)SmallStringReserve),
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
  char* s2 = str.bufferSlice().ptr;

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
    Logger::Warning("length (%d) must be greater or equal zero", limit);
    return false;
  }
  unsigned long plength = limit;

  z_stream stream;
  stream.zalloc = (alloc_func) Z_NULL;
  stream.zfree = (free_func) Z_NULL;

  unsigned long length = 0;
  int status;

  // We reallocate with an expanding factor, but want to start with a
  // smaller factor on larger strings to hope not to hit the max
  // string size as fast.
  unsigned int factor = len < 128 * 1024 * 1024 ? 4 : 2;
  unsigned int maxfactor = 16;

  String str(std::max(plength, (unsigned long)SmallStringReserve),
             ReserveString);
  do {
    if (length >= StringData::MaxSize) {
      return false;
    }

    length = plength ? plength : (unsigned long)len * (1 << factor++);
    length = std::min<unsigned long>(length, StringData::MaxSize);
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

Variant f_readgzfile(const String& filename, bool use_include_path /* = false */) {
  Resource stream = f_gzopen(filename, "rb", use_include_path);
  if (stream.isNull()) {
    return false;
  }
  return f_gzpassthru(stream);
}

Variant f_gzfile(const String& filename, bool use_include_path /* = false */) {
  Resource stream = f_gzopen(filename, "rb", use_include_path);
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

Variant f_gzcompress(const String& data, int level /* = -1 */) {
  return gzcompress(data.data(), data.size(), level);
}

Variant f_gzuncompress(const String& data, int limit /* = 0 */) {
  return gzuncompress(data.data(), data.size(), limit);
}

Variant f_gzdeflate(const String& data, int level /* = -1 */) {
  return gzdeflate(data.data(), data.size(), level);
}

Variant f_gzinflate(const String& data, int limit /* = 0 */) {
  return gzinflate(data.data(), data.size(), limit);
}

Variant f_gzencode(const String& data, int level /* = -1 */,
                   int encoding_mode /* = k_FORCE_GZIP */) {
  int len = data.size();
  char *ret = gzencode(data.data(), len, level, encoding_mode);
  if (ret == NULL) {
    return false;
  }
  return String(ret, len, AttachString);
}

Variant f_gzdecode(const String& data) {
  int len = data.size();
  char *ret = gzdecode(data.data(), len);
  if (ret == NULL) {
    return false;
  }
  return String(ret, len, AttachString);
}

String f_zlib_get_coding_type() {
  throw NotSupportedException(__func__, "no use");
}

///////////////////////////////////////////////////////////////////////////////
// stream functions

Resource f_gzopen(const String& filename, const String& mode,
                  bool use_include_path /* = false */) {
  File *file = NEWOBJ(ZipFile)();
  Resource handle(file);
  bool ret = file->open(File::TranslatePath(filename), mode);
  if (!ret) {
    raise_warning("%s",folly::errnoStr(errno).c_str());
    return NULL;
  }
  return handle;
}

bool f_gzclose(CResRef zp) {
  return f_fclose(zp);
}
Variant f_gzread(CResRef zp, int64_t length /* = 0 */) {
  return f_fread(zp, length);
}
Variant f_gzseek(CResRef zp, int64_t offset, int64_t whence /* = k_SEEK_SET */) {
  return f_fseek(zp, offset, whence);
}
Variant f_gztell(CResRef zp) {
  return f_ftell(zp);
}
bool f_gzeof(CResRef zp) {
  return f_feof(zp);
}
bool f_gzrewind(CResRef zp) {
  return f_rewind(zp);
}
Variant f_gzgetc(CResRef zp) {
  return f_fgetc(zp);
}
Variant f_gzgets(CResRef zp, int64_t length /* = 1024 */) {
  return f_fgets(zp, length);
}
Variant f_gzgetss(CResRef zp, int64_t length /* = 0 */,
                  const String& allowable_tags /* = null_string */) {
  return f_fgetss(zp, length, allowable_tags);
}
Variant f_gzpassthru(CResRef zp) {
  return f_fpassthru(zp);
}
Variant f_gzputs(CResRef zp, const String& str, int64_t length /* = 0 */) {
  return f_fwrite(zp, str, length);
}
Variant f_gzwrite(CResRef zp, const String& str, int64_t length /* = 0 */) {
  return f_fwrite(zp, str, length);
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
#include "hphp/runtime/ext/quicklz.inc"
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
#include "hphp/runtime/ext/quicklz.inc"
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
#include "hphp/runtime/ext/quicklz.inc"
}

#endif // HAVE_QUICKLZ

Variant f_qlzcompress(const String& data, int level /* = 1 */) {
#ifndef HAVE_QUICKLZ
  throw NotSupportedException(__func__, "QuickLZ library cannot be found");
#else
  if (level < 1 || level > 3) {
    throw_invalid_argument("level: %d", level);
    return false;
  }

  String str(data.size() + 400, ReserveString);
  char* compressed = str.bufferSlice().ptr;
  size_t size = 0;

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

  assert(size <= (size_t)data.size() + 400);
  return str.shrink(size);
#endif
}

Variant f_qlzuncompress(const String& data, int level /* = 1 */) {
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
  if ((int64_t)size < 0 ||
      (RuntimeOption::SerializationSizeLimit > 0 &&
       (int64_t)size > RuntimeOption::SerializationSizeLimit)) {
    raise_notice("invalid size in compressed header: %zd", size);
    return false;
  }

  String s = String(size, ReserveString);
  char *decompressed = s.bufferSlice().ptr;
  size_t dsize = 0;

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

  assert(dsize == size);
  return s.setSize(dsize);
#endif
}

Variant f_sncompress(const String& data) {
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

Variant f_snuncompress(const String& data) {
#ifndef HAVE_SNAPPY
  throw NotSupportedException(__func__, "Snappy library cannot be found");
#else
  size_t dsize;

  snappy::GetUncompressedLength(data.data(), data.size(), &dsize);
  String s = String(dsize, ReserveString);
  char *uncompressed = s.bufferSlice().ptr;

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

Variant f_nzcompress(const String& uncompressed) {
  size_t len = compressBound(uncompressed.size());
  String str(sizeof(nzlib_format_t) + len, ReserveString);
  nzlib_format_t* format = (nzlib_format_t*)str.bufferSlice().ptr;

  format->magic = htonl(NZLIB_MAGIC);
  format->uncompressed_sz = htonl(uncompressed.size());

  int rc = compress(format->buf, &len, (uint8_t*)uncompressed.data(),
                    uncompressed.size());
  if (rc == Z_OK) {
    return str.shrink(len + sizeof(*format));
  }
  return false;
}

Variant f_nzuncompress(const String& compressed) {
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
  char* uncompressed = str.bufferSlice().ptr;
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

Variant f_lz4compress(const String& uncompressed) {
  int bufsize = LZ4_compressBound(uncompressed.size());
  if (bufsize < 0) {
    return false;
  }
  int headerSize = VarintSize(uncompressed.size());
  bufsize += headerSize;  // for the header
  String s = String(bufsize, ReserveString);
  char *compressed = s.bufferSlice().ptr;

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

Variant f_lz4hccompress(const String& uncompressed) {
  int bufsize = LZ4_compressBound(uncompressed.size());
  if (bufsize < 0) {
    return false;
  }
  int headerSize = VarintSize(uncompressed.size());
  bufsize += headerSize;  // for the header
  String s = String(bufsize, ReserveString);
  char *compressed = s.bufferSlice().ptr;

  VarintEncode(uncompressed.size(), &compressed);  // write the header

  int csize = LZ4_compressHC(uncompressed.data(),
      compressed, uncompressed.size());
  if (csize < 0) {
    return false;
  }
  bufsize = csize + headerSize;
  return s.shrink(bufsize);
}

Variant f_lz4uncompress(const String& compressed) {
  const char* compressed_ptr = compressed.data();
  int dsize = VarintDecode(&compressed_ptr, compressed.size());
  if (dsize < 0) {
    return false;
  }

  String s = String(dsize, ReserveString);
  char *uncompressed = s.bufferSlice().ptr;
  int ret = LZ4_uncompress(compressed_ptr, uncompressed, dsize);

  if (ret <= 0) {
    return false;
  }
  return s.setSize(dsize);
}

///////////////////////////////////////////////////////////////////////////////
}
