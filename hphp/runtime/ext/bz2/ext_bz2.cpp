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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/file-stream-wrapper.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/stream-wrapper.h"
#include "hphp/runtime/ext/bz2/bz2-file.h"
#include "hphp/runtime/ext/std/ext_std_file.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/util/alloc.h"
#include <folly/String.h>

// Don't do the do { ... } while(0) trick here because we need 'f' outside of
// the macro
#define CHECK_BZFILE(handle, f)                               \
  auto f = dyn_cast_or_null<BZ2File>(handle);                 \
  if (f == nullptr || f->isClosed()) {                        \
    raise_warning("Not a valid stream resource");             \
    return (false);                                           \
  }

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// compress.zlib:// stream wrapper

struct BZ2StreamWrapper final : Stream::Wrapper {
  req::ptr<File>
  open(const String& filename, const String& mode, int /*options*/,
       const req::ptr<StreamContext>& /*context*/) override {
    static const char cz[] = "compress.bzip2://";

    if (strncmp(filename.c_str(), cz, sizeof(cz) - 1)) {
      assertx(false);
      return nullptr;
    }

    String fname(filename.substr(sizeof(cz) - 1));
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

    auto file = req::make<BZ2File>();
    if (!file->open(translated, mode)) {
      raise_warning("%s", file->getLastError().c_str());
      return nullptr;
    }
    return file;
  }
};

static BZ2StreamWrapper s_bzip2_stream_wrapper;

///////////////////////////////////////////////////////////////////////////////

bool HHVM_FUNCTION(bzclose, const OptResource& bz) {
  return HHVM_FN(fclose)(bz);
}

Variant HHVM_FUNCTION(bzread, const OptResource& bz, int64_t length /* = 1024 */) {
  return HHVM_FN(fread)(bz, length);
}

Variant HHVM_FUNCTION(bzwrite, const OptResource& bz, const String& data,
                               int64_t length /* = 0 */) {
  return HHVM_FN(fwrite)(bz, data, length);
}

const StaticString s_r("r"), s_w("w");

Variant HHVM_FUNCTION(bzopen, const Variant& filename, const String& mode) {
  if (mode != s_r && mode != s_w) {
    raise_warning(
      "'%s' is not a valid mode for bzopen(). "
      "Only 'w' and 'r' are supported.",
      mode.data()
    );
    return false;
  }

  req::ptr<BZ2File> bz;
  if (filename.isString()) {
    if (filename.asCStrRef().empty()) {
      raise_warning("filename cannot be empty");
      return false;
    } else if (!FileUtil::isValidPath(filename.asCStrRef())) {
      return false;
    }
    bz = req::make<BZ2File>();
    bool ret = bz->open(File::TranslatePath(filename.toString()), mode);
    if (!ret) {
      raise_warning("%s", folly::errnoStr(errno).c_str());
      return false;
    }
  } else {
    if (!filename.isResource()) {
      raise_warning("first parameter has to be string or file-resource");
      return false;
    }
    auto f = cast<PlainFile>(filename);
    if (!f) {
      return false;
    }

    std::string stream_mode = f->getMode();
    int stream_mode_len = stream_mode.length();

    if (stream_mode_len != 1 &&
        !(stream_mode_len == 2 &&
          stream_mode.find('b') != std::string::npos)) {
      raise_warning("cannot use stream opened in mode '%s'",
                    stream_mode.c_str());
      return false;
    } else if (stream_mode_len == 1 &&
        stream_mode[0] != 'r' && stream_mode[0] != 'w' &&
        stream_mode[0] != 'a' && stream_mode[0] != 'x') {
      raise_warning("cannot use stream opened in mode '%s'",
                    stream_mode.c_str());
      return false;
    }

    const char rw_mode = stream_mode[0];
    if (mode == s_r && rw_mode != 'r') {
      raise_warning("cannot write to a stream opened in read only mode");
      return false;
    } else if (mode == s_w && rw_mode != 'w' && rw_mode != 'a' &&
               rw_mode != 'x') {
      raise_warning("cannot read from a stream opened in write only mode");
      return false;
    }

    bz = req::make<BZ2File>(std::move(f));
  }
  return Variant(std::move(bz));
}

bool HHVM_FUNCTION(bzflush, const OptResource& bz) {
  CHECK_BZFILE(bz, f);
  return f->flush();
}

Variant HHVM_FUNCTION(bzerrstr, const OptResource& bz) {
  CHECK_BZFILE(bz, f);
  return f->errstr();
}

Variant HHVM_FUNCTION(bzerror, const OptResource& bz) {
  CHECK_BZFILE(bz, f);
  return f->error();
}

Variant HHVM_FUNCTION(bzerrno, const OptResource& bz) {
  CHECK_BZFILE(bz, f);
  return f->errnu();
}

Variant HHVM_FUNCTION(bzcompress, const String& source, int64_t blocksize /* = 4 */,
                                  int64_t workfactor /* = 0 */) {
  unsigned int source_len, dest_len;

  source_len = source.length();
  dest_len = source.length() + source.length() / 64 + 601;

  auto ret = String(dest_len, ReserveString);

  int error = BZ2_bzBuffToBuffCompress(ret.mutableData(), &dest_len,
                                   (char*)source.c_str(), source_len,
                                   blocksize, 0, workfactor);
  if (error != BZ_OK) {
    return error;
  } else {
    ret.setSize(dest_len);
    ret.shrink(dest_len);
    return ret;
  }
}

Variant HHVM_FUNCTION(bzdecompress, const String& source, int64_t small /* = 0 */) {
  int source_len = source.length();
  int error;
  uint64_t size = 0;
  bz_stream bzs;

  bzs.bzalloc = nullptr;
  bzs.bzfree = nullptr;

  if (BZ2_bzDecompressInit(&bzs, 0, small) != BZ_OK) {
    return false;
  }

  bzs.next_in = (char *) source.c_str();
  bzs.avail_in = source_len;

  // in most cases bz2 offers at least 2:1 compression, so we use that as our
  // base
  bzs.avail_out = source_len * 2;
  String ret(bzs.avail_out, ReserveString);
  bzs.next_out = ret.mutableData();

  while ((error = BZ2_bzDecompress(&bzs)) == BZ_OK && bzs.avail_in > 0) {
    /* compression is better then 2:1, need to allocate more memory */
    bzs.avail_out = source_len;
    size = (bzs.total_out_hi32 * (unsigned int) -1) + bzs.total_out_lo32;
    ret.setSize(size); // needs to be null-terminated before the reserve call
    bzs.next_out = ret.reserve(size + bzs.avail_out).data() + size;
  }

  if (error == BZ_STREAM_END || error == BZ_OK) {
    size = (bzs.total_out_hi32 * (unsigned int) -1) + bzs.total_out_lo32;
    BZ2_bzDecompressEnd(&bzs);
    ret.shrink(size);
    return ret;
  } else {
    BZ2_bzDecompressEnd(&bzs);
    return error;
  }
}

const StaticString s_SystemLib_ChunkedBunzipper(
  "__SystemLib\\ChunkedBunzipper");

struct ChunkedBunzipper {
  ChunkedBunzipper(): m_eof(false) {
    m_bzstream.bzalloc = nullptr;
    m_bzstream.bzfree = nullptr;
    int status = BZ2_bzDecompressInit(&m_bzstream, 0, 0);
    if (status != BZ_OK) {
      raise_error("Failed BZ2_bzDecompressInit: %d", status);
    }
  }

  ~ChunkedBunzipper() {
    if (!eof()) {
      BZ2_bzDecompressEnd(&m_bzstream);
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
    m_bzstream.next_in = (char *) chunk.data();
    m_bzstream.avail_in = chunk.length();
    unsigned int offset = 0;
    String result(1024 * 1024, ReserveString);
    int status;
    bool completed = false;
    for (int i = 0; i < 20; i++) {
      char* raw = result.mutableData() + offset;
      unsigned int avail_len = result.capacity() - offset;
      m_bzstream.next_out = raw;
      m_bzstream.avail_out = avail_len;

      status = BZ2_bzDecompress(&m_bzstream);
      if (BZ_STREAM_END == status || BZ_OK == status) {
        unsigned int produced = avail_len - m_bzstream.avail_out;
        offset += produced;
        result.setSize(offset);
        // Unlike zlib, bz2 doesn't always try to fill the output buffer
        // If the status is BZ_OK, we have to keep looping
        if (!produced || BZ_STREAM_END == status) {
          completed = true;
          break;
        }
        // If less than half available space was used in this loop, don't resize
        if (avail_len < produced * 2) {
          result.reserve(result.capacity() + 1);  // bump to next allocation size
        }
      } else {
        m_eof = true;
        BZ2_bzDecompressEnd(&m_bzstream);
        throw_object(
          "Exception",
          make_vec_array(folly::sformat("bz2 error status={}", status))
        );
        return empty_string();
      }
    }

    if (BZ_STREAM_END == status) {
      m_eof = true;
      BZ2_bzDecompressEnd(&m_bzstream);
    }
    if (!completed) {
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
      BZ2_bzDecompressEnd(&m_bzstream);
    }
  }

 private:
  ::bz_stream m_bzstream;
  bool m_eof;

  // z_stream contains void* that we don't care about.
  TYPE_SCAN_IGNORE_FIELD(m_bzstream);
};

#define FETCH_CHUNKED_BUNZIPPER(dest, src) \
  auto dest = Native::data<ChunkedBunzipper>(src);

bool HHVM_METHOD(ChunkedBunzipper, eof) {
  FETCH_CHUNKED_BUNZIPPER(data, this_);
  assertx(data);
  return data->eof();
}

String HHVM_METHOD(ChunkedBunzipper,
                   inflateChunk,
                   const String& chunk) {
  FETCH_CHUNKED_BUNZIPPER(data, this_);
  assertx(data);
  return data->inflateChunk(chunk);
}

void HHVM_METHOD(ChunkedBunzipper, close) {
  FETCH_CHUNKED_BUNZIPPER(data, this_);
  assertx(data);
  return data->close();
}

///////////////////////////////////////////////////////////////////////////////

struct bz2Extension final : Extension {
  bz2Extension() : Extension("bz2", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}

  void moduleLoad(const IniSetting::Map& /*ini*/, Hdf /*hdf*/) override {
    s_bzip2_stream_wrapper.registerAs("compress.bzip2");
  }

  void moduleRegisterNative() override {
    HHVM_FE(bzclose);
    HHVM_FE(bzread);
    HHVM_FE(bzwrite);
    HHVM_FE(bzopen);
    HHVM_FE(bzflush);
    HHVM_FE(bzerrstr);
    HHVM_FE(bzerror);
    HHVM_FE(bzerrno);
    HHVM_FE(bzcompress);
    HHVM_FE(bzdecompress);

    HHVM_NAMED_ME(__SystemLib\\ChunkedBunzipper, eof,
                  HHVM_MN(ChunkedBunzipper, eof));
    HHVM_NAMED_ME(__SystemLib\\ChunkedBunzipper, inflateChunk,
                  HHVM_MN(ChunkedBunzipper, inflateChunk));
    HHVM_NAMED_ME(__SystemLib\\ChunkedBunzipper, close,
                  HHVM_MN(ChunkedBunzipper, close));
    Native::registerNativeDataInfo<ChunkedBunzipper>(
      s_SystemLib_ChunkedBunzipper.get());
  }
} s_bz2_extension;

///////////////////////////////////////////////////////////////////////////////
}
