/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_COMPRESSION_H_
#define incl_HPHP_COMPRESSION_H_

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

class StreamCompressor {
public:
  StreamCompressor(int level, int encoding_mode, bool header);
  ~StreamCompressor();

  /**
   * Compress one chunk a time.
   */
  char *compress(const char *data, int &len, bool trailer);

private:
  int m_encoding;
  bool m_header;
  z_stream m_stream;
  uLong m_crc;
  bool m_ended;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif  // incl_HPHP_COMPRESSION_H_
