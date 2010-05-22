/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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
  while (!(line = f_gzgets(stream)).isNull()) {
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
  File *file = NEW(ZipFile)();
  Object handle(file);
  bool ret = file->open(File::TranslatePath(filename), mode);
  if (!ret) {
    raise_warning("%s",Util::safe_strerror(errno).c_str());
    return false;
  }
  return handle;
}

///////////////////////////////////////////////////////////////////////////////
}
