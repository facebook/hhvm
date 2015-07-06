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

#include "hphp/runtime/ext/bz2/bz2-file.h"
#include "hphp/runtime/ext/std/ext_std_file.h"
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

bool HHVM_FUNCTION(bzclose, const Resource& bz) {
  return HHVM_FN(fclose)(bz);
}

Variant HHVM_FUNCTION(bzread, const Resource& bz, int length /* = 1024 */) {
  return HHVM_FN(fread)(bz, length);
}

Variant HHVM_FUNCTION(bzwrite, const Resource& bz, const String& data,
                               int length /* = 0 */) {
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

bool HHVM_FUNCTION(bzflush, const Resource& bz) {
  CHECK_BZFILE(bz, f);
  return f->flush();
}

Variant HHVM_FUNCTION(bzerrstr, const Resource& bz) {
  CHECK_BZFILE(bz, f);
  return f->errstr();
}

Variant HHVM_FUNCTION(bzerror, const Resource& bz) {
  CHECK_BZFILE(bz, f);
  return f->error();
}

Variant HHVM_FUNCTION(bzerrno, const Resource& bz) {
  CHECK_BZFILE(bz, f);
  return f->errnu();
}

Variant HHVM_FUNCTION(bzcompress, const String& source, int blocksize /* = 4 */,
                                  int workfactor /* = 0 */) {
  char *dest = NULL;
  int error;
  unsigned int source_len, dest_len;

  source_len = source.length();
  dest_len = source.length() + (0.01*source.length()) + 600;

  if (!(dest = (char *)malloc(dest_len + 1))) {
    return BZ_MEM_ERROR;
  }

  error = BZ2_bzBuffToBuffCompress(dest, &dest_len, (char *) source.c_str(),
                                   source_len, blocksize, 0, workfactor);
  if (error != BZ_OK) {
    free(dest);
    return error;
  } else {
    // this is to shrink the allocation, since we probably over allocated
    dest = (char *)realloc(dest, dest_len + 1);
    dest[dest_len] = '\0';
    String ret = String(dest, dest_len, AttachString);
    return ret;
  }
}

Variant HHVM_FUNCTION(bzdecompress, const String& source, int small /* = 0 */) {
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
    bzs.next_out = ret.reserve(size + bzs.avail_out).ptr + size;
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

///////////////////////////////////////////////////////////////////////////////

class bz2Extension final : public Extension {
 public:
  bz2Extension() : Extension("bz2") {}
  void moduleInit() override {
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

    loadSystemlib("bz2");
  }
} s_bz2_extension;

///////////////////////////////////////////////////////////////////////////////
}
