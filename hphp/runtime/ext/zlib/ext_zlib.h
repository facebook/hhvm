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

#ifndef incl_HPHP_EXT_ZLIB_H_
#define incl_HPHP_EXT_ZLIB_H_

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/ext_file.h"

namespace HPHP {

extern const long k_FORCE_GZIP;
extern const long k_FORCE_DEFLATE;

///////////////////////////////////////////////////////////////////////////////
// zlib functions

Variant HHVM_FUNCTION(readgzfile, const String& filename,
                                  int64_t use_include_path = 0);
Variant HHVM_FUNCTION(gzfile, const String& filename,
                              int64_t use_include_path = 0);
Variant HHVM_FUNCTION(gzcompress, const String& data,
                                  int64_t level = -1);
Variant HHVM_FUNCTION(gzuncompress, const String& data,
                                    int limit = 0);
Variant HHVM_FUNCTION(gzdeflate, const String& data, int level = -1);
Variant HHVM_FUNCTION(gzinflate, const String& data, int limit = 0);
Variant HHVM_FUNCTION(gzencode, const String& data, int level = -1,
                                int encoding_mode = k_FORCE_GZIP);
Variant HHVM_FUNCTION(gzdecode, const String& data, int limit = 0);
String HHVM_FUNCTION(zlib_get_coding_type);
#ifdef HAVE_QUICKLZ
Variant HHVM_FUNCTION(qlzcompress, const String& data, int level = 1);
Variant HHVM_FUNCTION(qlzuncompress, const String& data, int level = 1);
#endif
#ifdef HAVE_SNAPPY
Variant HHVM_FUNCTION(sncompress, const String& data);
Variant HHVM_FUNCTION(snuncompress, const String& data);
#endif
Variant HHVM_FUNCTION(nzcompress, const String& uncompressed);
Variant HHVM_FUNCTION(nzuncompress, const String& compressed);
Variant HHVM_FUNCTION(lz4compress, const String& uncompressed);
Variant HHVM_FUNCTION(lz4hccompress, const String& uncompressed);
Variant HHVM_FUNCTION(lz4uncompress, const String& compressed);

///////////////////////////////////////////////////////////////////////////////
// stream functions

Variant HHVM_FUNCTION(gzopen, const String& filename, const String& mode,
                              int64_t use_include_path = 0);
bool HHVM_FUNCTION(gzclose, const Resource& zp);
Variant HHVM_FUNCTION(gzread, const Resource& zp, int64_t length = 0);
Variant HHVM_FUNCTION(gzseek, const Resource& zp, int64_t offset,
                              int64_t whence = k_SEEK_SET);
Variant HHVM_FUNCTION(gztell, const Resource& zp);
bool HHVM_FUNCTION(gzeof, const Resource& zp);
bool HHVM_FUNCTION(gzrewind, const Resource& zp);
Variant HHVM_FUNCTION(gzgetc, const Resource& zp);
Variant HHVM_FUNCTION(gzgets, const Resource& zp, int64_t length = 1024);
Variant HHVM_FUNCTION(gzgetss, const Resource& zp, int64_t length = 0,
                            const String& allowable_tags = null_string);
Variant HHVM_FUNCTION(gzpassthru, const Resource& zp);
Variant HHVM_FUNCTION(gzwrite, const Resource& zp, const String& str,
                               int64_t length = 0);

///////////////////////////////////////////////////////////////////////////////

}

#endif // incl_HPHP_EXT_ZLIB_H_
