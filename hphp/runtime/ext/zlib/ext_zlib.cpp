/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/zlib/ext_zlib.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/mem-file.h"
#include "hphp/runtime/ext/zlib/zip-file.h"
#include "hphp/runtime/base/stream-wrapper.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/file-stream-wrapper.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/util/compression.h"
#include "hphp/util/logger.h"
#include <folly/String.h>
#ifdef HAVE_SNAPPY
#include <snappy.h>
#endif
#include <lz4.h>
#include <lz4hc.h>
#include <memory>
#include <algorithm>

#define PHP_ZLIB_MODIFIER 1000


using namespace HPHP;

namespace {

///////////////////////////////////////////////////////////////////////////////
// compress.zlib:// stream wrapper

static class ZlibStreamWrapper : public Stream::Wrapper {
 public:
  virtual req::ptr<File> open(const String& filename,
                              const String& mode,
                              int options,
                              const req::ptr<StreamContext>& context) {
    String fname;
    static const char cz[] = "compress.zlib://";

    if (!strncmp(filename.data(), "zlib:", sizeof("zlib:") - 1)) {
      fname = filename.substr(sizeof("zlib:") - 1);
    } else if (!strncmp(filename.data(), cz, sizeof(cz) - 1)) {
      fname = filename.substr(sizeof(cz) - 1);
    } else {
      return nullptr;
    }

    String translated;
    if (fname.find("://") == -1) {
      translated = File::TranslatePath(fname);
      if (auto file = FileStreamWrapper::openFromCache(translated, mode)) {
        file->unzip();
        return file;
      }
    } else {
      translated = fname;
    }

    auto file = req::make<ZipFile>();
    bool ret = file->open(translated, mode);
    if (!ret) {
      raise_warning("%s", file->getLastError().c_str());
      return nullptr;
    }
    return file;
  }
} s_zlib_stream_wrapper;

} // nil namespace

namespace HPHP {

const int64_t k_ZLIB_ENCODING_RAW     = -MAX_WBITS;
const int64_t k_ZLIB_ENCODING_GZIP    = 0x1f;
const int64_t k_ZLIB_ENCODING_DEFLATE = 0x0f;

const int64_t k_ZLIB_ENCODING_ANY     = 0x2f;

const int64_t k_FORCE_GZIP            = k_ZLIB_ENCODING_GZIP;
const int64_t k_FORCE_DEFLATE         = k_ZLIB_ENCODING_DEFLATE;

///////////////////////////////////////////////////////////////////////////////
// zlib functions

Variant HHVM_FUNCTION(readgzfile, const String& filename,
                                  int64_t use_include_path /* = 0 */) {
  Variant stream = HHVM_FN(gzopen)(filename, "rb", use_include_path);
  if (stream.isBoolean() && !stream.toBoolean()) {
    return false;
  }
  return HHVM_FN(gzpassthru)(stream.toResource());
}

Variant HHVM_FUNCTION(gzfile, const String& filename,
                              int64_t use_include_path /* = 0 */) {
  Variant stream = HHVM_FN(gzopen)(filename, "rb", use_include_path);
  if (stream.isBoolean() && !stream.toBoolean()) {
    return false;
  }

  Array ret;
  Variant line;
  while (!same(line = HHVM_FN(gzgets)(stream.toResource()), false)) {
    ret.append(line);
  }
  return ret;
}

/////////////////////////////////////////////////////////////////////////////

inline size_t hhvm_zlib_buffer_size_guess(size_t inlen) {
  return ((double) inlen * (double) 1.015) + 23;
}

static voidpf hhvm_zlib_alloc(voidpf opaque, uInt items, uInt size) {
  return (voidpf)req::malloc(items * size);
}

static void hhvm_zlib_free(voidpf opaque, voidpf address) {
  req::free((void*)address);
}

/////////////////////////////////////////////////////////////////////////////

static Variant hhvm_zlib_encode(const String& data,
                                int64_t level, int64_t enc) {
  if ((level < -1) || (level > 9)) {
    raise_warning("compression level (%" PRId64 ") must be within -1..9",
                  level);
    return false;
  }
  switch (enc) {
    case k_ZLIB_ENCODING_RAW:
    case k_ZLIB_ENCODING_GZIP:
    case k_ZLIB_ENCODING_DEFLATE:
      break;
    default:
      raise_warning("encoding mode must be either ZLIB_ENCODING_RAW, "
                    "ZLIB_ENCODING_GZIP or ZLIB_ENCODING_DEFLATE");
      return false;
  }

  z_stream Z;
  memset(&Z, 0, sizeof(z_stream));
  Z.zalloc = (alloc_func) hhvm_zlib_alloc;
  Z.zfree = (free_func) hhvm_zlib_free;

  int status;
  if (Z_OK == (status = deflateInit2(&Z, level, Z_DEFLATED, enc,
                                     MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY))) {
    SCOPE_EXIT { deflateEnd(&Z); };
    size_t outlen = hhvm_zlib_buffer_size_guess(data.size());
    String ret(outlen, ReserveString);

    Z.next_in = (Bytef *) data.c_str();
    Z.next_out = (Bytef *) ret.mutableData();
    Z.avail_in = data.size();
    Z.avail_out = ret.capacity(); // not counting null terminator

    if (Z_STREAM_END == (status = deflate(&Z, Z_FINISH))) {
      ret.setSize(Z.total_out);
      return ret;
    }
  }

  raise_warning("%s", zError(status));
  return false;
}

Variant HHVM_FUNCTION(zlib_encode, const String& data,
                                          int64_t encoding,
                                          int64_t level /*= -1 */) {
  return hhvm_zlib_encode(data, level, encoding);
}
Variant HHVM_FUNCTION(gzcompress, const String& data,
                                  int64_t level) {
  return hhvm_zlib_encode(data, level, k_ZLIB_ENCODING_DEFLATE);
}
Variant HHVM_FUNCTION(gzdeflate, const String& data, int level) {
  return hhvm_zlib_encode(data, level, k_ZLIB_ENCODING_RAW);
}
Variant HHVM_FUNCTION(gzencode, const String& data, int level,
                                int encoding_mode) {
  return hhvm_zlib_encode(data, level, encoding_mode);
}

/////////////////////////////////////////////////////////////////////////////

/* Expand a zlib stream into a String
 *
 * Starts with an optimistically sized output string of
 * input size or maxlen, whichever is less.
 *
 * From there, grows by ~12.5% per iteration to account for expansion.
 *
 * Runs at most 100 times, if we haven't finished by then,
 * call it a data error to avoid going nuts.
 */
static String hhvm_zlib_inflate_rounds(z_stream *Z, int64_t maxlen,
                                       int &status) {
  size_t retsize = (maxlen && maxlen < Z->avail_in) ? maxlen : Z->avail_in;
  String ret(retsize + 1, ReserveString);
  size_t retused = 0;
  int round = 0;

  do {
    if (maxlen && (maxlen < retused)) {
      status = Z_MEM_ERROR;
      break;
    }

    auto const ms = ret.reserve(retsize + 1);
    char *retbuf = ms.ptr;
    Z->avail_out = ms.len - retused;
    Z->next_out = (Bytef *) (retbuf + retused);
    status = inflate(Z, Z_NO_FLUSH);
    retused = ms.len - Z->avail_out;
    ret.setSize(retused);

    retsize += (retsize >> 3) + 1;
  } while ((Z_BUF_ERROR == status || (Z_OK == status && Z->avail_in)) &&
           ++round < 100);

  if (Z_STREAM_END == status) {
    return ret;
  }
  if (Z_OK == status) {
    status = Z_DATA_ERROR;
  }
  return String();
}

static Variant hhvm_zlib_decode(const String& data,
                                int64_t maxlen, int64_t enc) {
  if (data.empty()) {
    raise_warning("%s", zError(Z_DATA_ERROR));
    return false;
  }

  if (maxlen < 0) {
    raise_warning("length (%" PRId64 ") must be greater or equal zero", maxlen);
    return false;
  }

  z_stream Z;
  memset(&Z, 0, sizeof(z_stream));
  Z.zalloc = (alloc_func) hhvm_zlib_alloc;
  Z.zfree = (free_func) hhvm_zlib_free;

retry_raw_inflate:
  int status = inflateInit2(&Z, enc);
  if (Z_OK == status) {
    Z.next_in = (Bytef*)data.c_str();
    Z.avail_in = data.size() + 1;
    String ret = hhvm_zlib_inflate_rounds(&Z, maxlen, status);
    if (status == Z_STREAM_END) {
      inflateEnd(&Z);
      return ret;
    }
    if ((status == Z_DATA_ERROR) &&
        (k_ZLIB_ENCODING_ANY == enc)) {
       inflateEnd(&Z);
      enc = k_ZLIB_ENCODING_RAW;
      goto retry_raw_inflate;
    }
    inflateEnd(&Z);
  }

  raise_warning("%s", zError(status));
  return false;
}

Variant HHVM_FUNCTION(zlib_decode, const String& data,
                                   int64_t limit) {
  return hhvm_zlib_decode(data, limit, k_ZLIB_ENCODING_ANY);
}
Variant HHVM_FUNCTION(gzuncompress, const String& data,
                                    int limit) {
  return hhvm_zlib_decode(data, limit, k_ZLIB_ENCODING_DEFLATE);
}
Variant HHVM_FUNCTION(gzinflate, const String& data, int limit) {
  return hhvm_zlib_decode(data, limit, k_ZLIB_ENCODING_RAW);
}
Variant HHVM_FUNCTION(gzdecode, const String& data, int limit) {
  return hhvm_zlib_decode(data, limit, k_ZLIB_ENCODING_GZIP);
}

/////////////////////////////////////////////////////////////////////////////

String HHVM_FUNCTION(zlib_get_coding_type) {
  throw_not_supported(__func__, "no use");
}

///////////////////////////////////////////////////////////////////////////////
// stream functions

Variant HHVM_FUNCTION(gzopen, const String& filename, const String& mode,
                              int64_t use_include_path /* = 0 */) {
  auto file = req::make<ZipFile>();
  bool ret = file->open(File::TranslatePath(filename), mode);
  if (!ret) {
    return false;
  }
  return Variant(std::move(file));
}

bool HHVM_FUNCTION(gzclose, const Resource& zp) {
  return HHVM_FN(fclose)(zp);
}
Variant HHVM_FUNCTION(gzread, const Resource& zp, int64_t length /* = 0 */) {
  return HHVM_FN(fread)(zp, length);
}
Variant HHVM_FUNCTION(gzseek, const Resource& zp, int64_t offset,
                              int64_t whence /* = k_SEEK_SET */) {
  return HHVM_FN(fseek)(zp, offset, whence);
}
Variant HHVM_FUNCTION(gztell, const Resource& zp) {
  return HHVM_FN(ftell)(zp);
}
bool HHVM_FUNCTION(gzeof, const Resource& zp) {
  return HHVM_FN(feof)(zp);
}
bool HHVM_FUNCTION(gzrewind, const Resource& zp) {
  return HHVM_FN(rewind)(zp);
}
Variant HHVM_FUNCTION(gzgetc, const Resource& zp) {
  return HHVM_FN(fgetc)(zp);
}
Variant HHVM_FUNCTION(gzgets, const Resource& zp, int64_t length /* = 1024 */) {
  return HHVM_FN(fgets)(zp, length);
}
Variant HHVM_FUNCTION(gzgetss, const Resource& zp, int64_t length /* = 0 */,
                            const String& allowable_tags /* = null_string */) {
  return HHVM_FN(fgetss)(zp, length, allowable_tags);
}
Variant HHVM_FUNCTION(gzpassthru, const Resource& zp) {
  return HHVM_FN(fpassthru)(zp);
}
Variant HHVM_FUNCTION(gzwrite, const Resource& zp, const String& str,
                               int64_t length /* = 0 */) {
  return HHVM_FN(fwrite)(zp, str, length);
}

///////////////////////////////////////////////////////////////////////////////
// Snappy functions

#ifdef HAVE_SNAPPY

Variant HHVM_FUNCTION(snappy_compress, const String& data) {
  size_t size;
  char *compressed =
    (char *)malloc(snappy::MaxCompressedLength(data.size()) + 1);

  snappy::RawCompress(data.data(), data.size(), compressed, &size);
  compressed = (char *)realloc(compressed, size + 1);
  compressed[size] = '\0';
  return String(compressed, size, AttachString);
}

Variant HHVM_FUNCTION(snappy_uncompress, const String& data) {
  size_t dsize;

  snappy::GetUncompressedLength(data.data(), data.size(), &dsize);
  String s = String(dsize, ReserveString);
  char *uncompressed = s.mutableData();

  if (!snappy::RawUncompress(data.data(), data.size(), uncompressed)) {
    return false;
  }
  s.setSize(dsize);
  return s;
}
#endif // HAVE_SNAPPY

///////////////////////////////////////////////////////////////////////////////
// NZLIB functions

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

Variant HHVM_FUNCTION(nzcompress, const String& uncompressed) {
  size_t len = compressBound(uncompressed.size());
  String str(sizeof(nzlib_format_t) + len, ReserveString);
  nzlib_format_t* format = (nzlib_format_t*)str.mutableData();

  format->magic = htonl(NZLIB_MAGIC);
  format->uncompressed_sz = htonl(uncompressed.size());

  int rc = compress(format->buf, &len, (uint8_t*)uncompressed.data(),
                    uncompressed.size());
  if (rc == Z_OK) {
    str.shrink(len + sizeof(*format));
    return str;
  }
  return false;
}

Variant HHVM_FUNCTION(nzuncompress, const String& compressed) {
  if (compressed.size() < (ssize_t)sizeof(nzlib_format_t)) {
    return false;
  }

  nzlib_format_t* format = (nzlib_format_t*)compressed.data();
  if (ntohl(format->magic) != NZLIB_MAGIC) {
    return false;
  }

  size_t len = ntohl(format->uncompressed_sz);
  if (len == 0) {
    return empty_string_variant();
  }

  String str(len, ReserveString);
  char* uncompressed = str.mutableData();
  int rc = uncompress((Bytef*)uncompressed, &len, format->buf,
                      compressed.size() - sizeof(*format));
  if (rc != Z_OK) {
    return false;
  }

  if (format->uncompressed_sz == 0) {
    str.shrink(len);
    return str;
  }
  str.setSize(len);
  return str;
}

///////////////////////////////////////////////////////////////////////////////
// LZ4 functions

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

Variant HHVM_FUNCTION(lz4_compress, const String& uncompressed,
                                    bool high /* = false */) {
  if (high) {
    return f_lz4_hccompress(uncompressed);
  }
  int bufsize = LZ4_compressBound(uncompressed.size());
  if (bufsize < 0) {
    return false;
  }
  int headerSize = VarintSize(uncompressed.size());
  bufsize += headerSize;  // for the header
  String s = String(bufsize, ReserveString);
  char *compressed = s.mutableData();

  VarintEncode(uncompressed.size(), &compressed);  // write the header

  int csize = LZ4_compress(uncompressed.data(), compressed,
      uncompressed.size());
  if (csize < 0) {
    return false;
  }
  bufsize = csize + headerSize;
  s.shrink(bufsize);
  return s;
}

Variant HHVM_FUNCTION(lz4_hccompress, const String& uncompressed) {
  int bufsize = LZ4_compressBound(uncompressed.size());
  if (bufsize < 0) {
    return false;
  }
  int headerSize = VarintSize(uncompressed.size());
  bufsize += headerSize;  // for the header
  String s = String(bufsize, ReserveString);
  char *compressed = s.mutableData();

  VarintEncode(uncompressed.size(), &compressed);  // write the header

  int csize = LZ4_compressHC(uncompressed.data(),
      compressed, uncompressed.size());
  if (csize < 0) {
    return false;
  }
  bufsize = csize + headerSize;
  s.shrink(bufsize);
  return s;
}

Variant HHVM_FUNCTION(lz4_uncompress, const String& compressed) {
  const char* compressed_ptr = compressed.data();
  int dsize = VarintDecode(&compressed_ptr, compressed.size());
  if (dsize < 0) {
    return false;
  }

  int inSize = compressed.size() - (compressed_ptr - compressed.data());
  String s = String(dsize, ReserveString);
  char *uncompressed = s.mutableData();
#ifdef LZ4_MAX_INPUT_SIZE
  int ret = LZ4_decompress_safe(compressed_ptr, uncompressed, inSize, dsize);

  if (ret != dsize) {
    return false;
  }
#else
  int ret = LZ4_decompress_fast(compressed_ptr, uncompressed, dsize);

  if (ret <= 0 || ret > inSize) {
    return false;
  }
#endif

  s.setSize(dsize);
  return s;
}

///////////////////////////////////////////////////////////////////////////////
// Chunk-based API

const StaticString s___SystemLib_ChunkedInflator("__SystemLib_ChunkedInflator");

class __SystemLib_ChunkedInflator {
 public:
  __SystemLib_ChunkedInflator(): m_eof(false) {
    m_zstream.zalloc = (alloc_func) Z_NULL;
    m_zstream.zfree = (free_func) Z_NULL;
    int status = inflateInit2(&m_zstream, -MAX_WBITS);
    if (status != Z_OK) {
      raise_error("Failed to init zlib: %d", status);
    }
  }

  ~__SystemLib_ChunkedInflator() {
    if (!eof()) {
      inflateEnd(&m_zstream);
    }
  }

  bool eof() const {
    return m_eof;
  }

  String inflateChunk(const String& chunk) {
    if (m_eof) {
      raise_warning("Tried to inflate after final chunk");
      return empty_string();
    }
    m_zstream.next_in = (Bytef*) chunk.data();
    m_zstream.avail_in = chunk.length();
    int factor = chunk.length() < 128 * 1024 * 1024 ? 4 : 2;
    unsigned int maxfactor = 16;
    do {
      int buffer_length = chunk.length() * (1 << factor);
      String buffer(buffer_length, ReserveString);
      char* raw = buffer.mutableData();
      m_zstream.next_out = (Bytef*) raw;
      m_zstream.avail_out = buffer_length;

      int status = inflate(&m_zstream, Z_SYNC_FLUSH);
      if (status == Z_STREAM_END || status == Z_OK) {
        if (status == Z_STREAM_END) {
          m_eof = true;
        }
        int64_t produced = buffer_length - m_zstream.avail_out;
        if (produced) {
          buffer.shrink(produced);
          return buffer;
        }
        return empty_string();
      }
    } while (++factor < maxfactor);
    raise_warning("Failed to extract chunk");
    return empty_string();
  }

 private:
  ::z_stream m_zstream;
  bool m_eof;
};

#define FETCH_CHUNKED_INFLATOR(dest, src) \
  auto dest = Native::data<__SystemLib_ChunkedInflator>(src);

bool HHVM_METHOD(__SystemLib_ChunkedInflator, eof) {
  FETCH_CHUNKED_INFLATOR(data, this_);
  assert(data);
  return data->eof();
}

String HHVM_METHOD(__SystemLib_ChunkedInflator,
                   inflateChunk,
                   const String& chunk) {
  FETCH_CHUNKED_INFLATOR(data, this_);
  assert(data);
  return data->inflateChunk(chunk);
}

///////////////////////////////////////////////////////////////////////////////

class ZlibExtension final : public Extension {
 public:
  ZlibExtension() : Extension("zlib", "2.0") {}
  void moduleLoad(const IniSetting::Map& ini, Hdf hdf) override {
    s_zlib_stream_wrapper.registerAs("compress.zlib");
  }
  void moduleInit() override {
#define X(cns) \
    Native::registerConstant<KindOfInt64>(makeStaticString(#cns), k_##cns);
    X(ZLIB_ENCODING_RAW);
    X(ZLIB_ENCODING_GZIP);
    X(ZLIB_ENCODING_DEFLATE);
    X(ZLIB_ENCODING_ANY);
    X(FORCE_GZIP);
    X(FORCE_DEFLATE);
#undef X

    HHVM_FE(zlib_encode);
    HHVM_FE(gzdeflate);
    HHVM_FE(gzcompress);
    HHVM_FE(gzencode);

    HHVM_FE(zlib_decode);
    HHVM_FE(gzinflate);
    HHVM_FE(gzuncompress);
    HHVM_FE(gzdecode);

    HHVM_FE(readgzfile);
    HHVM_FE(gzfile);
    HHVM_FE(zlib_get_coding_type);
    HHVM_FE(gzopen);
    HHVM_FE(gzclose);
    HHVM_FE(gzread);
    HHVM_FE(gzseek);
    HHVM_FE(gztell);
    HHVM_FE(gzeof);
    HHVM_FE(gzrewind);
    HHVM_FE(gzgetc);
    HHVM_FE(gzgets);
    HHVM_FE(gzgetss);
    HHVM_FE(gzpassthru);
    HHVM_FE(gzwrite);
#ifdef HAVE_QUICKLZ
    HHVM_FE(qlzcompress);
    HHVM_FE(qlzuncompress);
#endif
#ifdef HAVE_SNAPPY
    HHVM_FE(snappy_compress);
    HHVM_FE(snappy_uncompress);
#endif
    HHVM_FE(nzcompress);
    HHVM_FE(nzuncompress);
    HHVM_FE(lz4_compress);
    HHVM_FE(lz4_hccompress);
    HHVM_FE(lz4_uncompress);

    HHVM_ME(__SystemLib_ChunkedInflator, eof);
    HHVM_ME(__SystemLib_ChunkedInflator, inflateChunk);

    Native::registerNativeDataInfo<__SystemLib_ChunkedInflator>(
      s___SystemLib_ChunkedInflator.get());

    loadSystemlib();
#ifdef HAVE_QUICKLZ
    loadSystemlib("zlib-qlz");
#endif
#ifdef HAVE_SNAPPY
    loadSystemlib("zlib-snappy");
#endif
  }
} s_zlib_extension;
///////////////////////////////////////////////////////////////////////////////
}
