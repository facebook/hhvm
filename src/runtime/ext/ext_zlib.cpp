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
  if (level < -1 || level > 9) {
    throw_invalid_argument("level: %d", level);
    return false;
  }
  int len = data.size();
  char *ret = gzcompress(data.data(), len, level);
  if (ret == NULL) {
    return false;
  }
  return String(ret, len, AttachString);
}

Variant f_gzuncompress(CStrRef data, int limit /* = 0 */) {
  int len = data.size();
  char *ret = gzuncompress(data.data(), len, limit);
  if (ret == NULL) {
    return false;
  }
  return String(ret, len, AttachString);
}

Variant f_gzdeflate(CStrRef data, int level /* = -1 */) {
  if (level < -1 || level > 9) {
    throw_invalid_argument("level: %d", level);
    return false;
  }
  int len = data.size();
  char *ret = gzdeflate(data.data(), len, level);
  if (ret == NULL) {
    return false;
  }
  return String(ret, len, AttachString);
}

Variant f_gzinflate(CStrRef data, int limit /* = 0 */) {
  int len = data.size();
  char *ret = gzinflate(data.data(), len, limit);
  if (ret == NULL) {
    return false;
  }
  return String(ret, len, AttachString);
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

  char *compressed = (char*)malloc(data.size() + 401);
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

  ASSERT(size < (size_t)data.size() + 401);
  compressed = (char *)realloc(compressed, size + 1);
  compressed[size] = '\0';
  return String(compressed, size, AttachString);
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
  nzlib_format_t* format = NULL;
  size_t len = 0;
  char *compressed = NULL;
  int rc;

  len = compressBound(uncompressed.size());
  format = (nzlib_format_t*)malloc(sizeof(*format) + len);
  if (format == NULL) {
    goto error;
  }

  format->magic = htonl(NZLIB_MAGIC);
  format->uncompressed_sz = htonl(uncompressed.size());

  rc = compress(format->buf, &len, (uint8_t*)uncompressed.data(),
                uncompressed.size());
  if (rc != Z_OK) {
    goto error;
  }

  compressed = (char*)realloc(format, len + sizeof(*format) + 1);
  if (compressed == NULL) {
    goto error;
  }
  compressed[len + sizeof(*format)] = '\0';
  return String(compressed, len + sizeof(*format), AttachString);

error:
  free(format);
  return false;
}

Variant f_nzuncompress(CStrRef compressed) {
  char *uncompressed = NULL;
  size_t len = 0;
  nzlib_format_t* format = NULL;
  int rc;

  if (compressed.size() < (ssize_t)sizeof(*format)) {
    goto error;
  }

  format = (nzlib_format_t*)compressed.data();
  if (ntohl(format->magic) != NZLIB_MAGIC) {
    goto error;
  }

  len = ntohl(format->uncompressed_sz);
  if (len == 0) {
    return String("", AttachLiteral);
  }

  uncompressed = (char*)malloc(len + 1);
  if (uncompressed == NULL) {
    goto error;
  }
  rc = uncompress((Bytef*)uncompressed, &len, format->buf,
                  compressed.size() - sizeof(*format));
  if (rc != Z_OK) {
    goto error;
  }

  if (format->uncompressed_sz == 0) {
    char *p = (char*)realloc(uncompressed, len + 1);
    if (p == NULL) {
      goto error;
    }
    uncompressed = p;
  }
  uncompressed[len] = '\0';
  return String(uncompressed, len, AttachString);

error:
  free(uncompressed);
  return false;
}

Variant f_lz4compress(CStrRef data) {
  int bufsize = LZ4_compressBound(data.size());
  if (bufsize < 0) {
    return false;
  }
  bufsize += sizeof(int);  // for the header
  String s = String(bufsize, ReserveString);
  char *compressed = s.mutableSlice().ptr;

  *((int*)compressed) = data.size();  // write the header

  int csize = LZ4_compress(data.data(), compressed + sizeof(int), data.size());
  if (csize < 0) {
    return false;
  }
  bufsize = csize + sizeof(int);
  s.shrink(bufsize);
  return s.setSize(bufsize);
}

Variant f_lz4hccompress(CStrRef data) {
  int bufsize = LZ4_compressBound(data.size());
  if (bufsize < 0) {
    return false;
  }
  bufsize += sizeof(int);  // for the header
  String s = String(bufsize, ReserveString);
  char *compressed = s.mutableSlice().ptr;

  *((int*)compressed) = data.size();  // write the header

  int csize = LZ4_compressHC(data.data(),
      compressed + sizeof(int), data.size());
  if (csize < 0) {
    return false;
  }
  bufsize = csize + sizeof(int);
  return s.shrink(bufsize);
}

Variant f_lz4uncompress(CStrRef data) {
  if (data.size() < (ssize_t)sizeof(int)) {
    return false;
  }
  int dsize = *((int*)data.data());
  if (dsize < 0) {
    return false;
  }

  String s = String(dsize, ReserveString);
  char *uncompressed = s.mutableSlice().ptr;
  int ret = LZ4_uncompress(data.data() + sizeof(int), uncompressed, dsize);

  if (ret <= 0) {
    return false;
  }
  return s.setSize(dsize);
}

///////////////////////////////////////////////////////////////////////////////
}
