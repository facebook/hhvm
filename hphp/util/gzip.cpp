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

#include "hphp/util/gzip.h"

#include "hphp/util/alloc.h"
#include "hphp/util/configs/server.h"
#include "hphp/util/exception.h"
#include "hphp/util/logger.h"

#define PHP_ZLIB_MODIFIER 1000
#define GZIP_HEADER_LENGTH 10
#define GZIP_FOOTER_LENGTH 8

namespace HPHP {

namespace {

void* local_zalloc(void* /* opaque */, unsigned items, unsigned size) {
  auto const bytes = static_cast<size_t>(items) *  size;
  if (bytes == 0) return nullptr;
  return local_malloc(bytes);
}

void local_zfree(void* /* opaque */, void* p) {
  if (p) local_free(p);
}

}

static const int gz_magic[2] = {0x1f, 0x8b}; /* gzip magic header */
///////////////////////////////////////////////////////////////////////////////
// This check_header() function is copied from zlib 1.2.3 and re-factored to
// work with in-memory buffers (than file streams).

/* zlib.h -- interface of the 'zlib' general purpose compression library
  version 1.2.3, July 18th, 2005

  Copyright (C) 1995-2005 Jean-loup Gailly and Mark Adler

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Jean-loup Gailly        Mark Adler
  jloup@gzip.org          madler@alumni.caltech.edu


  The data format used by the zlib library is described by RFCs (Request for
  Comments) 1950 to 1952 in the files http://www.ietf.org/rfc/rfc1950.txt
  (zlib format), rfc1951.txt (deflate format) and rfc1952.txt (gzip format).
*/

/* gzip flag byte */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define HEAD_CRC     0x02 /* bit 1 set: header CRC present */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define RESERVED     0xE0 /* bits 5..7: reserved */

/* ===========================================================================
     Read a byte from a gz_stream; update next_in and avail_in. Return EOF
   for end of file.
   IN assertion: the stream s has been successfully opened for reading.
*/
static int get_byte(z_stream &stream) {
  if (stream.avail_in == 0) {
    return EOF;
  }
  stream.avail_in--;
  return *(stream.next_in)++;
}

/* ===========================================================================
      Check the gzip header of a gz_stream opened for reading. Set the stream
    mode to transparent if the gzip magic header is not present; set s->err
    to Z_DATA_ERROR if the magic header is present but the rest of the header
    is incorrect.
    IN assertion: the stream s has already been created successfully;
       s->stream.avail_in is zero for the first time, but may be non-zero
       for concatenated .gz files.
*/
static int check_header(z_stream &stream) {
  int method; /* method byte */
  int flags; /* flags byte */
  uInt len;
  int c;

  /* Assure two bytes in the buffer so we can peek ahead -- handle case
    where first byte of header is at the end of the buffer after the last
    gzip segment */
  len = stream.avail_in;
  if (len <= 2) {
    return Z_DATA_ERROR;
  }

  /* Peek ahead to check the gzip magic header */
  if (stream.next_in[0] != gz_magic[0] || stream.next_in[1] != gz_magic[1]) {
    return Z_DATA_ERROR;
  }
  stream.avail_in -= 2;
  stream.next_in += 2;

  /* Check the rest of the gzip header */
  method = get_byte(stream);
  flags = get_byte(stream);
  if (method != Z_DEFLATED || (flags & RESERVED) != 0) {
    return Z_DATA_ERROR;
  }

  /* Discard time, xflags and OS code: */
  for (len = 0; len < 6; len++) (void)get_byte(stream);

  if ((flags & EXTRA_FIELD) != 0) { /* skip the extra field */
    len = (uInt)get_byte(stream);
    len += ((uInt)get_byte(stream))<<8;
    /* len is garbage if EOF but the loop below will quit anyway */
    while (len-- != 0 && get_byte(stream) != EOF) ;
  }
  if ((flags & ORIG_NAME) != 0) { /* skip the original file name */
    while ((c = get_byte(stream)) != 0 && c != EOF) ;
  }
  if ((flags & COMMENT) != 0) {  /* skip the .gz file comment */
    while ((c = get_byte(stream)) != 0 && c != EOF) ;
  }
  if ((flags & HEAD_CRC) != 0) { /* skip the header crc */
    for (len = 0; len < 2; len++) (void)get_byte(stream);
  }
  return stream.avail_in == 0 ? Z_DATA_ERROR : Z_OK;
}

///////////////////////////////////////////////////////////////////////////////

bool is_compressible_file(const char *filename) {
  static const char *ext[] = {
    "gif", "png", "jpeg", "jpg", "tiff", "swf", "zip", "gz", "bz2", "cab",
    "bmp", "xcf", "mp3", "wav", "rsrc", "ico", "jar", "exe", "dll", "so",
  };
  const char *dot = nullptr;
  for (const char *p = filename; *p; p++) {
    if (*p == '.') dot = p;
  }
  if (dot) {
    dot++;
    for (unsigned int i = 0; i < sizeof(ext)/sizeof(ext[0]); i++) {
      if (strcmp(dot, ext[i]) == 0) {
        return false;
      }
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// GzipCompressor

GzipCompressor::GzipCompressor(int level, int encoding_mode, bool header)
  : m_encoding(encoding_mode), m_header(header),
    m_ended(false) {
  if (level < -1 || level > 9) {
    throw Exception("compression level(%d) must be within -1..9", level);
  }
  if (encoding_mode != CODING_GZIP && encoding_mode != CODING_DEFLATE) {
    throw Exception("encoding mode must be FORCE_GZIP or FORCE_DEFLATE");
  }

  if (Cfg::Server::GzipUseLocalArena) {
    m_stream.zalloc = local_zalloc;
    m_stream.zfree = local_zfree;
  } else {
    m_stream.zalloc = Z_NULL;
    m_stream.zfree = Z_NULL;
  }
  m_stream.opaque = Z_NULL;
  m_stream.total_in = 0;
  m_stream.next_in = Z_NULL;
  m_stream.avail_in = 0;
  m_stream.avail_out = 0;
  m_stream.next_out = Z_NULL;

  m_crc = crc32(0L, Z_NULL, 0);

  int status;
  switch (encoding_mode) {
  case CODING_GZIP:
    /* windowBits is passed < 0 to suppress zlib header & trailer */
    if ((status = deflateInit2(&m_stream, level, Z_DEFLATED, -MAX_WBITS,
                               MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY)) != Z_OK) {
      throw Exception("%s", zError(status));
    }
    break;
  case CODING_DEFLATE:
    if ((status = deflateInit(&m_stream, level)) != Z_OK) {
      throw Exception("%s", zError(status));
    }
    break;
  }
}

GzipCompressor::~GzipCompressor() {
  if (!m_ended) {
    deflateEnd(&m_stream);
  }
}

StringHolder
GzipCompressor::compress(const char *data, int &len, bool trailer) {
  // middle chunks should never be zero size
  assert(len || trailer);

  m_stream.next_in = (Bytef *)data;
  m_stream.avail_in = len;
  m_stream.total_out = 0;

  m_stream.avail_out = m_stream.avail_in +
    (m_stream.avail_in / PHP_ZLIB_MODIFIER) + 15 + 1; /* room for \0 */
  char *s2;
  auto const allocSize = m_stream.avail_out + GZIP_HEADER_LENGTH +
    ((trailer && m_encoding == CODING_GZIP) ? GZIP_FOOTER_LENGTH : 0);
  if (Cfg::Server::GzipUseLocalArena) {
    s2 = (char *)local_malloc(allocSize);
  } else {
    s2 = (char *)malloc(allocSize);
  }

  /* add gzip file header */
  bool header = m_header;
  if (header) {
    s2[0] = (char)gz_magic[0];
    s2[1] = (char)gz_magic[1];
    s2[2] = Z_DEFLATED;
    s2[3] = s2[4] = s2[5] = s2[6] = s2[7] = s2[8] = 0; /* time set to 0 */
    s2[9] = 0x03; // OS_CODE
    m_stream.next_out = (Bytef*)&(s2[GZIP_HEADER_LENGTH]);
    m_header = false; // only the 1st chunk got it
  } else {
    m_stream.next_out = (Bytef*)s2;
  }

  int status = deflate(&m_stream, trailer ? Z_FINISH : Z_SYNC_FLUSH);
  if (status == Z_BUF_ERROR || status == Z_STREAM_END) {
    status = deflateEnd(&m_stream);
    m_ended = true;
  }
  if (status == Z_OK) {
    if (len) {
      m_crc = crc32(m_crc, (const Bytef *)data, len);
    }
    int new_len = m_stream.total_out + (header ? GZIP_HEADER_LENGTH : 0);
    len = new_len;
    if (trailer && m_encoding == CODING_GZIP) {
      len += GZIP_FOOTER_LENGTH;
      char *strailer = s2 + new_len;

      /* write crc & stream.total_in in LSB order */
      strailer[0] = (char) m_crc & 0xFF;
      strailer[1] = (char) (m_crc >> 8) & 0xFF;
      strailer[2] = (char) (m_crc >> 16) & 0xFF;
      strailer[3] = (char) (m_crc >> 24) & 0xFF;
      strailer[4] = (char) m_stream.total_in & 0xFF;
      strailer[5] = (char) (m_stream.total_in >> 8) & 0xFF;
      strailer[6] = (char) (m_stream.total_in >> 16) & 0xFF;
      strailer[7] = (char) (m_stream.total_in >> 24) & 0xFF;
      strailer[8] = '\0';
    } else {
      s2[len] = '\0';
    }
    return StringHolder(s2, len, Cfg::Server::GzipUseLocalArena ? FreeType::LocalFree
                                                                : FreeType::Free);
  }
  if (Cfg::Server::GzipUseLocalArena) {
    local_free(s2);
  } else {
    free(s2);
  }
  Logger::Error("%s", zError(status));
  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

char *gzencode(const char *data, int &len, int level, int encoding_mode) {
  if (level < -1 || level > 9) {
    Logger::Warning("compression level(%d) must be within -1..9", level);
    return nullptr;
  }

  if (encoding_mode != CODING_GZIP && encoding_mode != CODING_DEFLATE) {
    Logger::Warning("encoding mode must be FORCE_GZIP or FORCE_DEFLATE");
    return nullptr;
  }

  z_stream stream;
  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;

  stream.next_in = (Bytef *)data;
  stream.avail_in = len;

  stream.avail_out = stream.avail_in + (stream.avail_in / PHP_ZLIB_MODIFIER) +
    15 + 1; /* room for \0 */
  char *s2 = (char *)malloc
    (stream.avail_out + GZIP_HEADER_LENGTH +
     (encoding_mode == CODING_GZIP ? GZIP_FOOTER_LENGTH : 0));
  if (!s2) {
    return nullptr;
  }
  /* add gzip file header */
  s2[0] = (char)gz_magic[0];
  s2[1] = (char)gz_magic[1];
  s2[2] = Z_DEFLATED;
  s2[3] = s2[4] = s2[5] = s2[6] = s2[7] = s2[8] = 0; /* time set to 0 */
  s2[9] = 0x03; // OS_CODE

  stream.next_out = (Bytef*)&(s2[GZIP_HEADER_LENGTH]);

  int status;
  switch (encoding_mode) {
  case CODING_GZIP:
    /* windowBits is passed < 0 to suppress zlib header & trailer */
    if ((status = deflateInit2(&stream, level, Z_DEFLATED, -MAX_WBITS,
                               MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY)) != Z_OK) {
      Logger::Warning("%s", zError(status));
      return nullptr;
    }
    break;
  case CODING_DEFLATE:
    if ((status = deflateInit(&stream, level)) != Z_OK) {
      Logger::Warning("%s", zError(status));
      return nullptr;
    }
    break;
  }

  status = deflate(&stream, Z_FINISH);
  if (status != Z_STREAM_END) {
    deflateEnd(&stream);
    if (status == Z_OK) {
      status = Z_BUF_ERROR;
    }
  } else {
    status = deflateEnd(&stream);
  }

  if (status == Z_OK) {

    int old_len = len;
    len = stream.total_out + GZIP_HEADER_LENGTH +
      (encoding_mode == CODING_GZIP ? GZIP_FOOTER_LENGTH : 0);
    /* resize to buffer to the "right" size */
    s2 = (char *)realloc(s2, len + 1);
    assert(s2);
    if (encoding_mode == CODING_GZIP) {
      char *trailer = s2 + (stream.total_out + GZIP_HEADER_LENGTH);
      uLong crc = crc32(0L, Z_NULL, 0);

      crc = crc32(crc, (const Bytef *)data, old_len);

      /* write crc & stream.total_in in LSB order */
      trailer[0] = (char) crc & 0xFF;
      trailer[1] = (char) (crc >> 8) & 0xFF;
      trailer[2] = (char) (crc >> 16) & 0xFF;
      trailer[3] = (char) (crc >> 24) & 0xFF;
      trailer[4] = (char) stream.total_in & 0xFF;
      trailer[5] = (char) (stream.total_in >> 8) & 0xFF;
      trailer[6] = (char) (stream.total_in >> 16) & 0xFF;
      trailer[7] = (char) (stream.total_in >> 24) & 0xFF;
      trailer[8] = '\0';
    } else {
      s2[len] = '\0';
    }
    return s2;
  }

  free(s2);
  Logger::Warning("%s", zError(status));
  return nullptr;
}

char *gzdecode(const char *data, int &len) {
  z_stream stream;
  stream.zalloc = (alloc_func) Z_NULL;
  stream.zfree = (free_func) Z_NULL;

  unsigned long length;
  int status;
  unsigned int factor = 4, maxfactor = 16;
  char *s1 = nullptr, *s2 = nullptr;
  do {
    stream.next_in = (Bytef *)data;
    stream.avail_in = (uInt)len + 1; /* there is room for \0 */
    if (check_header(stream) != Z_OK) {
      Logger::Warning("gzdecode: header is in wrong format");
      return nullptr;
    }

    length = len * (1 << factor++);
    s2 = (char *)realloc(s1, length);
    if (!s2) {
      if (s1) free(s1);
      return nullptr;
    }
    s1 = s2;

    stream.next_out = (Bytef*)s2;
    stream.avail_out = (uInt)length;

    /* init with -MAX_WBITS disables the zlib internal headers */
    status = inflateInit2(&stream, -MAX_WBITS);
    if (status == Z_OK) {
      status = inflate(&stream, Z_FINISH);
      if (status != Z_STREAM_END) {
        inflateEnd(&stream);
        if (status == Z_OK) {
          status = Z_BUF_ERROR;
        }
      } else {
        status = inflateEnd(&stream);
      }
    }
  } while (status == Z_BUF_ERROR && factor < maxfactor);

  if (status == Z_OK) {
    len = stream.total_out;

    // shrink the buffer down to what we really need since this can be 16
    // times greater than we actually need.
    s2 = (char *)realloc(s2, len + 1);
    assert(s2);
    s2[len] = '\0';
    return s2;
  }

  free(s2);
  Logger::Warning("%s", zError(status));
  return nullptr;
}

}
