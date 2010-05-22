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

#ifndef __EXT_ZLIB_H__
#define __EXT_ZLIB_H__

#include <runtime/base/base_includes.h>
#include <runtime/ext/ext_file.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// zlib functions

Variant f_readgzfile(CStrRef filename, bool use_include_path = false);
Variant f_gzfile(CStrRef filename, bool use_include_path = false);
Variant f_gzcompress(CStrRef data, int level = -1);
Variant f_gzuncompress(CStrRef data, int limit = 0);
Variant f_gzdeflate(CStrRef data, int level = -1);
Variant f_gzinflate(CStrRef data, int limit = 0);
Variant f_gzencode(CStrRef data, int level = -1,
                   int encoding_mode = k_FORCE_GZIP);
Variant f_gzdecode(CStrRef data);
inline String f_zlib_get_coding_type() {
  throw NotSupportedException(__func__, "no use");
}

///////////////////////////////////////////////////////////////////////////////
// stream functions

Object f_gzopen(CStrRef filename, CStrRef mode, bool use_include_path = false);

inline bool f_gzclose(CObjRef zp) {
  return f_fclose(zp);
}
inline Variant f_gzread(CObjRef zp, int64 length = 0) {
  return f_fread(zp, length);
}
inline Variant f_gzseek(CObjRef zp, int64 offset, int64 whence = SEEK_SET) {
  return f_fseek(zp, offset, whence);
}
inline Variant f_gztell(CObjRef zp) {
  return f_ftell(zp);
}
inline bool f_gzeof(CObjRef zp) {
  return f_feof(zp);
}
inline bool f_gzrewind(CObjRef zp) {
  return f_rewind(zp);
}
inline Variant f_gzgetc(CObjRef zp) {
  return f_fgetc(zp);
}
inline Variant f_gzgets(CObjRef zp, int64 length = 1024) {
  return f_fgets(zp, length);
}
inline Variant f_gzgetss(CObjRef zp, int64 length = 0,
                        CStrRef allowable_tags = null_string) {
  return f_fgetss(zp, length, allowable_tags);
}
inline Variant f_gzpassthru(CObjRef zp) {
  return f_fpassthru(zp);
}
inline Variant f_gzputs(CObjRef zp, CStrRef str, int64 length = 0) {
  return f_fwrite(zp, str, length);
}
inline Variant f_gzwrite(CObjRef zp, CStrRef str, int64 length = 0) {
  return f_fwrite(zp, str, length);
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_ZLIB_H__
