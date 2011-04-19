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

  char *decompressed = (char*)malloc(size + 1);
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
  decompressed[dsize] = '\0';
  return String(decompressed, dsize, AttachString);
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
  char *uncompressed;
  size_t dsize;

  snappy::GetUncompressedLength(data.data(), data.size(), &dsize);
  uncompressed = (char *)malloc(dsize + 1);

  if (!snappy::RawUncompress(data.data(), data.size(), uncompressed)) {
    free(uncompressed);
    return false;
  }
  uncompressed[dsize] = '\0';
  return String(uncompressed, dsize, AttachString);
#endif
}

///////////////////////////////////////////////////////////////////////////////
}
