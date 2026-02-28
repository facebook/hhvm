/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include "hphp/util/string-holder.h"
#include <zlib.h>

// encoding_mode
#define CODING_GZIP     1
#define CODING_DEFLATE  2

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// inefficient way of testing whether a file can be compressed to smaller size
bool is_compressible_file(const char *filename);

char *gzencode(const char *data, int &len, int level, int encoding_mode);
char *gzdecode(const char *data, int &len);

///////////////////////////////////////////////////////////////////////////////

// Supports producing GZIP- or DEFLATE-compatible output via the encoding_mode
// flag.
struct GzipCompressor {
  GzipCompressor(int level, int encoding_mode, bool header);
  ~GzipCompressor();

  /**
   * Compress one chunk a time.
   */
  StringHolder compress(const char *data, int &len, bool trailer);

 private:
  int m_encoding;
  bool m_header;
  z_stream m_stream;
  uLong m_crc;
  bool m_ended;
};

///////////////////////////////////////////////////////////////////////////////
}
