/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/mem-file.h"
#include "hphp/runtime/ext/zlib/zip-file.h"
#include "hphp/runtime/base/stream-wrapper.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/file-stream-wrapper.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/util/gzip.h"
#include "hphp/util/logger.h"
#include <folly/String.h>
#include <memory>
#include <algorithm>
#include <vector>

#define PHP_ZLIB_MODIFIER 1000

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// compress.zlib:// stream wrapper

static struct ZlibStreamWrapper final : Stream::Wrapper {
  req::ptr<File>
  open(const String& filename, const String& mode, int /*options*/,
       const req::ptr<StreamContext>& /*context*/) override {
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
      return nullptr;
    }
    return file;
  }
} s_zlib_stream_wrapper;

const int64_t k_ZLIB_ENCODING_RAW     = -MAX_WBITS;
const int64_t k_ZLIB_ENCODING_GZIP    = 0x1f;
const int64_t k_ZLIB_ENCODING_DEFLATE = 0x0f;

const int64_t k_ZLIB_ENCODING_ANY     = 0x2f;

const int64_t k_FORCE_GZIP            = k_ZLIB_ENCODING_GZIP;
const int64_t k_FORCE_DEFLATE         = k_ZLIB_ENCODING_DEFLATE;

/////////////////////////////////////////////////////////////////////////////

inline size_t hhvm_zlib_buffer_size_guess(size_t inlen) {
  return ((double) inlen * (double) 1.015) + 23;
}

static voidpf hhvm_zlib_alloc(voidpf /*opaque*/, uInt items, uInt size) {
  return (voidpf)req::malloc_noptrs(items * size);
}

static void hhvm_zlib_free(voidpf /*opaque*/, voidpf address) {
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
Variant HHVM_FUNCTION(gzdeflate, const String& data, int64_t level) {
  return hhvm_zlib_encode(data, level, k_ZLIB_ENCODING_RAW);
}
Variant HHVM_FUNCTION(gzencode, const String& data, int64_t level,
                                int64_t encoding_mode) {
  return hhvm_zlib_encode(data, level, encoding_mode);
}

/////////////////////////////////////////////////////////////////////////////

/* Expand a zlib stream into a String
 *
 * Starts with an optimistically sized output string of
 * input size or maxlen (must be >= 0), whichever is less.
 *
 * From there, grows by ~12.5% per iteration to account for expansion.
 *
 * Runs at most 100 times, if we haven't finished by then,
 * call it a data error to avoid going nuts.
 */
static String hhvm_zlib_inflate_rounds(z_stream *Z, int64_t maxlen,
                                       int &status) {
  assertx(maxlen >= 0);
  size_t retsize = (maxlen && maxlen < Z->avail_in) ? maxlen : Z->avail_in;
  String ret;
  size_t retused = 0;
  int round = 0;

  do {
    if (UNLIKELY(retsize >= kMaxSmallSize) &&
        UNLIKELY(tl_heap->preAllocOOM(retsize + 1))) {
      VMRegAnchor _;
      assertx(checkSurpriseFlags());
      handle_request_surprise();
    }

    auto const ms = [&]() -> folly::MutableStringPiece {
      if (!ret.get()) {
        ret = String(retsize + 1, ReserveString);
        return ret.bufferSlice();
      }
      return ret.reserve(retsize + 1);
    }();

    auto retbuf = ms.data();
    Z->avail_out = ms.size() - retused;
    Z->next_out = (Bytef *) (retbuf + retused);
    status = inflate(Z, Z_NO_FLUSH);
    retused = ms.size() - Z->avail_out;
    ret.setSize(retused);
    if (maxlen && (maxlen < retused)) {
      status = Z_MEM_ERROR;
      break;
    }
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
  SCOPE_EXIT { inflateEnd(&Z); };

  int status = inflateInit2(&Z, enc);
  if (Z_OK == status) {
    Z.next_in = (Bytef*)data.c_str();
    Z.avail_in = data.size() + 1;
    String ret = hhvm_zlib_inflate_rounds(&Z, maxlen, status);
    if (status == Z_STREAM_END) {
      return ret;
    }
    if ((status == Z_DATA_ERROR) &&
        (k_ZLIB_ENCODING_ANY == enc)) {
      enc = k_ZLIB_ENCODING_RAW;
      goto retry_raw_inflate;
    }
  }

  raise_warning("%s", zError(status));
  return false;
}

Variant HHVM_FUNCTION(zlib_decode, const String& data,
                                   int64_t limit) {
  return hhvm_zlib_decode(data, limit, k_ZLIB_ENCODING_ANY);
}
Variant HHVM_FUNCTION(gzuncompress, const String& data,
                                    int64_t limit) {
  return hhvm_zlib_decode(data, limit, k_ZLIB_ENCODING_DEFLATE);
}
Variant HHVM_FUNCTION(gzinflate, const String& data, int64_t limit) {
  return hhvm_zlib_decode(data, limit, k_ZLIB_ENCODING_RAW);
}
Variant HHVM_FUNCTION(gzdecode, const String& data, int64_t limit) {
  return hhvm_zlib_decode(data, limit, k_ZLIB_ENCODING_GZIP);
}

/////////////////////////////////////////////////////////////////////////////

String HHVM_FUNCTION(zlib_get_coding_type) {
  throw_not_supported(__func__, "no use");
}

///////////////////////////////////////////////////////////////////////////////
// stream functions

Variant HHVM_FUNCTION(gzopen, const String& filename, const String& mode,
                      int64_t /*use_include_path*/ /* = 0 */) {
  if (!FileUtil::checkPathAndWarn(filename, __FUNCTION__ + 2, 1)) {
    return init_null();
  }

  auto file = req::make<ZipFile>();
  bool ret = file->open(File::TranslatePath(filename), mode);
  if (!ret) {
    return false;
  }
  return Variant(std::move(file));
}

bool HHVM_FUNCTION(gzclose, const OptResource& zp) {
  return HHVM_FN(fclose)(zp);
}
Variant HHVM_FUNCTION(gzread, const OptResource& zp, int64_t length /* = 0 */) {
  return HHVM_FN(fread)(zp, length);
}
Variant HHVM_FUNCTION(gzseek, const OptResource& zp, int64_t offset,
                              int64_t whence /* = SEEK_SET */) {
  return HHVM_FN(fseek)(zp, offset, whence);
}
Variant HHVM_FUNCTION(gztell, const OptResource& zp) {
  return HHVM_FN(ftell)(zp);
}
bool HHVM_FUNCTION(gzeof, const OptResource& zp) {
  return HHVM_FN(feof)(zp);
}
bool HHVM_FUNCTION(gzrewind, const OptResource& zp) {
  return HHVM_FN(rewind)(zp);
}
Variant HHVM_FUNCTION(gzgetc, const OptResource& zp) {
  return HHVM_FN(fgetc)(zp);
}
Variant HHVM_FUNCTION(gzgets, const OptResource& zp, int64_t length /* = 0 */) {
  return HHVM_FN(fgets)(zp, length);
}
Variant HHVM_FUNCTION(gzgetss, const OptResource& zp, int64_t length /* = 0 */,
                            const String& allowable_tags /* = null_string */) {
  return HHVM_FN(fgetss)(zp, length, allowable_tags);
}
Variant HHVM_FUNCTION(gzpassthru, const OptResource& zp) {
  return HHVM_FN(fpassthru)(zp);
}
Variant HHVM_FUNCTION(gzwrite, const OptResource& zp, const String& str,
                               int64_t length /* = 0 */) {
  return HHVM_FN(fwrite)(zp, str, length);
}

///////////////////////////////////////////////////////////////////////////////
// zlib functions

Variant HHVM_FUNCTION(readgzfile, const String& filename,
                                  int64_t use_include_path /* = 0 */) {
  if (!FileUtil::checkPathAndWarn(filename, __FUNCTION__ + 2, 1)) {
    return init_null();
  }

  Variant stream = HHVM_FN(gzopen)(filename, "rb", use_include_path);
  if (stream.isBoolean() && !stream.toBoolean()) {
    return false;
  }
  return HHVM_FN(gzpassthru)(stream.toResource());
}

Variant HHVM_FUNCTION(gzfile, const String& filename,
                              int64_t use_include_path /* = 0 */) {
  if (!FileUtil::checkPathAndWarn(filename, __FUNCTION__ + 2, 1)) {
    return init_null();
  }

  Variant stream = HHVM_FN(gzopen)(filename, "rb", use_include_path);
  if (stream.isBoolean() && !stream.toBoolean()) {
    return false;
  }

  Array ret = Array::CreateVec();
  Variant line;
  while (!same(line = HHVM_FN(gzgets)(stream.toResource(), 0), false)) {
    ret.append(line);
  }
  return ret.empty() ? init_null() : ret;
}

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
  uLong len = compressBound(uncompressed.size());
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

  uLong len = ntohl(format->uncompressed_sz);
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
// Chunk-based API

const StaticString s_SystemLib_ChunkedInflator("__SystemLib\\ChunkedInflator");
const StaticString s_SystemLib_ChunkedGunzipper(
  "__SystemLib\\ChunkedGunzipper");

template<int W>
struct ChunkedDecompressor {
  ChunkedDecompressor(): m_eof(false) {
    m_zstream.zalloc = (alloc_func) Z_NULL;
    m_zstream.zfree = (free_func) Z_NULL;
    int status = inflateInit2(&m_zstream, W);
    if (status != Z_OK) {
      raise_error("Failed to init zlib: %d", status);
    }
  }

  ~ChunkedDecompressor() {
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
    unsigned int offset = 0;
    String result(1024 * 1024, ReserveString);
    int status;
    bool completed = false;
    for (int i = 0; i < 20; i++) {
      char* raw = result.mutableData() + offset;
      m_zstream.next_out = (Bytef*) raw;
      m_zstream.avail_out = result.capacity() - offset;
      status = inflate(&m_zstream, Z_SYNC_FLUSH);
      if (status == Z_STREAM_END || status == Z_OK || status == Z_BUF_ERROR) {
        if (status == Z_STREAM_END) {
          m_eof = true;
        }
        unsigned int produced = result.capacity() - offset - m_zstream.avail_out;
        offset += produced;
        result.setSize(offset);
        // from zlib doc https://www.zlib.net/manual.html
        // "if inflate returns Z_OK and with zero avail_out, it must be called
        // again after making room in the output buffer because there might be
        // more output pending"
        // "... Note that Z_BUF_ERROR is not fatal, and inflate() can be
        // called again with more input and more output space to continue
        // decompressing"
        if (m_zstream.avail_out > 0) {
          completed = true;
          break;
        }
        result.reserve(result.capacity() + 1);  // bump to next allocation size
      } else {
        m_eof = true;
        inflateEnd(&m_zstream);
        throw_object(
          "Exception",
          make_vec_array(
            folly::sformat("zlib error status={} msg=\"{}\"",
              status,
              m_zstream.msg
            )
          )
        );
        return empty_string();
      }
    }

    if (Z_STREAM_END == status) {
      m_eof = true;
      inflateEnd(&m_zstream);
    }
    if (!completed) {
      // output too large
      throw_object(
        "Exception",
        make_vec_array("inflate failed: output too large")
      );
      return empty_string();
    }
    return result;
  }

  void close() {
    if (!m_eof) {
      m_eof = true;
      inflateEnd(&m_zstream);
    }
  }

  int getUndecompressedByteCount() {
    return m_zstream.avail_in;
  }

 private:
  ::z_stream m_zstream;
  bool m_eof;

  // z_stream contains void* that we don't care about.
  TYPE_SCAN_IGNORE_FIELD(m_zstream);
};

// As per zlib manual (https://www.zlib.net/manual.html)
//  "... windowBits can also be -8..-15 for raw deflate ..."
//  "... windowBits can also be greater than 15 for optional gzip encoding.
//  Add 16 to windowBits to write a simple gzip header and trailer around
//  the compressed data instead of a zlib wrapper ..."
typedef ChunkedDecompressor<-MAX_WBITS> ChunkedInflator;
typedef ChunkedDecompressor<16 + MAX_WBITS> ChunkedGunzipper;

#define FETCH_CHUNKED_INFLATOR(dest, src) \
  auto dest = Native::data<ChunkedInflator>(src);

bool HHVM_METHOD(ChunkedInflator, eof) {
  FETCH_CHUNKED_INFLATOR(data, this_);
  assertx(data);
  return data->eof();
}

String HHVM_METHOD(ChunkedInflator,
                   inflateChunk,
                   const String& chunk) {
  FETCH_CHUNKED_INFLATOR(data, this_);
  assertx(data);
  return data->inflateChunk(chunk);
}

void HHVM_METHOD(ChunkedInflator, close) {
  FETCH_CHUNKED_INFLATOR(data, this_);
  assertx(data);
  return data->close();
}

int64_t HHVM_METHOD(ChunkedInflator, getUndecompressedByteCount) {
  FETCH_CHUNKED_INFLATOR(data, this_);
  assertx(data);
  return data->getUndecompressedByteCount();
}

#define FETCH_CHUNKED_GUNZIPPER(dest, src) \
  auto dest = Native::data<ChunkedGunzipper>(src);

bool HHVM_METHOD(ChunkedGunzipper, eof) {
  FETCH_CHUNKED_GUNZIPPER(data, this_);
  assertx(data);
  return data->eof();
}

String HHVM_METHOD(ChunkedGunzipper,
                   inflateChunk,
                   const String& chunk) {
  FETCH_CHUNKED_GUNZIPPER(data, this_);
  assertx(data);
  return data->inflateChunk(chunk);
}

void HHVM_METHOD(ChunkedGunzipper, close) {
  FETCH_CHUNKED_GUNZIPPER(data, this_);
  assertx(data);
  return data->close();
}

int64_t HHVM_METHOD(ChunkedGunzipper, getUndecompressedByteCount) {
  FETCH_CHUNKED_GUNZIPPER(data, this_);
  assertx(data);
  return data->getUndecompressedByteCount();
}

///////////////////////////////////////////////////////////////////////////////

struct ZlibExtension final : Extension {
  ZlibExtension() : Extension("zlib", "2.0", NO_ONCALL_YET) {}
  void moduleLoad(const IniSetting::Map& /*ini*/, Hdf /*hdf*/) override {
    s_zlib_stream_wrapper.registerAs("compress.zlib");
  }
  void moduleRegisterNative() override {
    HHVM_RC_INT(ZLIB_ENCODING_RAW, k_ZLIB_ENCODING_RAW);
    HHVM_RC_INT(ZLIB_ENCODING_GZIP, k_ZLIB_ENCODING_GZIP);
    HHVM_RC_INT(ZLIB_ENCODING_DEFLATE, k_ZLIB_ENCODING_DEFLATE);
    HHVM_RC_INT(ZLIB_ENCODING_ANY, k_ZLIB_ENCODING_ANY);
    HHVM_RC_INT(FORCE_GZIP, k_FORCE_GZIP);
    HHVM_RC_INT(FORCE_DEFLATE, k_FORCE_DEFLATE);

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
    HHVM_FE(nzcompress);
    HHVM_FE(nzuncompress);

    HHVM_NAMED_ME(__SystemLib\\ChunkedInflator, eof,
                  HHVM_MN(ChunkedInflator, eof));
    HHVM_NAMED_ME(__SystemLib\\ChunkedInflator, inflateChunk,
                  HHVM_MN(ChunkedInflator, inflateChunk));
    HHVM_NAMED_ME(__SystemLib\\ChunkedInflator, close,
                  HHVM_MN(ChunkedInflator, close));
    HHVM_NAMED_ME(__SystemLib\\ChunkedInflator, getUndecompressedByteCount,
                  HHVM_MN(ChunkedInflator, getUndecompressedByteCount));
    HHVM_NAMED_ME(__SystemLib\\ChunkedGunzipper, eof,
                  HHVM_MN(ChunkedGunzipper, eof));
    HHVM_NAMED_ME(__SystemLib\\ChunkedGunzipper, inflateChunk,
                  HHVM_MN(ChunkedGunzipper, inflateChunk));
    HHVM_NAMED_ME(__SystemLib\\ChunkedGunzipper, close,
                  HHVM_MN(ChunkedGunzipper, close));
    HHVM_NAMED_ME(__SystemLib\\ChunkedGunzipper, getUndecompressedByteCount,
                  HHVM_MN(ChunkedGunzipper, getUndecompressedByteCount));

    Native::registerNativeDataInfo<ChunkedInflator>(
      s_SystemLib_ChunkedInflator.get());

    Native::registerNativeDataInfo<ChunkedGunzipper>(
      s_SystemLib_ChunkedGunzipper.get());
  }
} s_zlib_extension;
///////////////////////////////////////////////////////////////////////////////
}
