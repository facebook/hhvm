/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/server/static-content-cache.h"
#include "hphp/runtime/server/virtual-host.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/util.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/zend-printf.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "folly/String.h"

#include <sys/file.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// statics

StaticString File::s_resource_name("stream");

IMPLEMENT_REQUEST_LOCAL(FileData, s_file_data);

const int File::USE_INCLUDE_PATH = 1;

String File::TranslatePathKeepRelative(const String& filename) {
  String canonicalized(
    Util::canonicalize(
      filename.data(),
      strlen(filename.data()) // canonicalize asserts that we don't have nulls
    ),
    AttachString);
  if (RuntimeOption::SafeFileAccess) {
    const vector<string> &allowedDirectories =
      VirtualHost::GetAllowedDirectories();
    auto it = std::upper_bound(allowedDirectories.begin(),
                               allowedDirectories.end(), canonicalized,
                               [](const String& val, const string& dir) {
                                 return strcmp(val.c_str(), dir.c_str()) < 0;
                               });
    if (it != allowedDirectories.begin()) {
      const string& dir = *--it;
      if (dir.size() <= canonicalized.size() &&
          !strncmp(dir.c_str(), canonicalized.c_str(), dir.size())) {
        return canonicalized;
      }
    }

    // disallow access with an absolute path
    if (canonicalized.charAt(0) == '/') {
      return "";
    }

    // unresolvable paths are all considered as unsafe
    if (canonicalized.find("..") >= 0) {
      assert(canonicalized.find("..") == 0);
      return "";
    }
  }

  return canonicalized;
}

String File::TranslatePath(const String& filename) {
  String canonicalized = TranslatePathKeepRelative(filename);

  if (canonicalized.charAt(0) == '/') {
    return canonicalized;
  }

  String cwd = g_context->getCwd();
  if (!cwd.empty() && cwd[cwd.length() - 1] == '/') {
    return cwd + canonicalized;
  }
  return cwd + "/" + canonicalized;
}

String File::TranslatePathWithFileCache(const String& filename) {
  String canonicalized(
    Util::canonicalize(
      filename.data(),
      strlen(filename.data()) // canonicalize asserts that we don't have nulls
    ),
    AttachString);
  String translated = TranslatePath(canonicalized);
  if (!translated.empty() && access(translated.data(), F_OK) < 0 &&
      StaticContentCache::TheFileCache) {
    if (StaticContentCache::TheFileCache->exists(canonicalized.data(),
                                                 false)) {
      // we use file cache's file name to make stat() work
      translated = String(RuntimeOption::FileCache);
    }
  }
  return translated;
}

String File::TranslateCommand(const String& cmd) {
  //TODO: security checking
  return cmd;
}

bool File::IsVirtualDirectory(const String& filename) {
  if (StaticContentCache::TheFileCache &&
      StaticContentCache::TheFileCache->dirExists(filename.data(), false)) {
    return true;
  }
  return false;
}

bool File::IsPlainFilePath(const String& filename) {
  return filename.find("://") == String::npos;
}

Variant File::Open(const String& filename, const String& mode,
                   int options /* = 0 */,
                   CVarRef context /* = null */) {
  Stream::Wrapper *wrapper = Stream::getWrapperFromURI(filename);
  File *file = wrapper->open(filename, mode, options, context);
  if (file != nullptr) {
    file->m_name = filename.data();
    file->m_mode = mode.data();
    return Resource(file);
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// constructor and destructor

File::File(bool nonblocking)
  : m_isLocal(false), m_fd(-1), m_closed(false), m_nonblocking(nonblocking),
    m_writepos(0), m_readpos(0), m_position(0), m_buffer(nullptr) {
}

File::~File() {
  closeImpl();
}

void File::sweep() {
  // Clear non-smart state without deleting `this`. Therefore assumes
  // `this` has been smart allocated. Note that the derived class'
  // sweep() is responsible for closing m_fd and any other non-smart
  // resources it might have allocated.
  assert(!valid());
  free(m_buffer);
  m_name.~string();
  m_mode.~string();
}

void File::closeImpl() {
  free(m_buffer);
  m_buffer = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// default implementation of virtual functions

int File::getc() {
  if (m_writepos > m_readpos) {
    m_position++;
    return m_buffer[m_readpos++] & 0xff;
  }

  char buffer[1];
  int64_t len = readImpl(buffer, 1);
  if (len != 1) {
    return EOF;
  }
  m_position += len;
  return (int)(unsigned char)buffer[0];
}

String File::read(int64_t length) {
  if (length <= 0) {
    raise_notice("Invalid length %" PRId64, length);
    return "";
  }

  String s = String(length, ReserveString);
  char *ret = s.bufferSlice().ptr;
  int64_t copied = 0;
  int64_t avail = bufferedLen();

  while (avail < length && !eof()) {
    if (m_buffer == nullptr) {
      m_buffer = (char *)malloc(CHUNK_SIZE);
    }

    if (avail > 0) {
      memcpy(ret + copied, m_buffer + m_readpos, avail);
      copied += avail;
      length -= avail;
    }

    m_writepos = readImpl(m_buffer, CHUNK_SIZE);
    m_readpos = 0;
    avail = bufferedLen();

    if (avail == 0 || m_nonblocking) {
      // For nonblocking mode, temporary out of data.
      break;
    }
  }

  avail = bufferedLen();
  if (avail > 0) {
    int64_t n = length < avail ? length : avail;
    memcpy(ret + copied, m_buffer + m_readpos, n);
    m_readpos += n;
    copied += n;
  }

  m_position += copied;
  return s.setSize(copied);
}

int64_t File::write(const String& data, int64_t length /* = 0 */) {
  if (seekable()) {
    int64_t offset = m_readpos - m_writepos;
    m_readpos = m_writepos = 0; // invalidating read buffer
    seek(offset, SEEK_CUR);
  }
  if (length <= 0 || length > data.size()) {
    length = data.size();
  }
  if (length) {
    int64_t written = writeImpl(data.data(), length);
    m_position += written;
    return written;
  }
  return 0;
}

int File::putc(char c) {
  char buf[1];
  buf[0] = c;
  int ret = writeImpl(buf, 1);
  m_position += ret;
  return ret;
}

bool File::seek(int64_t offset, int whence /* = SEEK_SET */) {
  if (whence != SEEK_CUR) {
    throw NotSupportedException(__func__, "cannot seek other than SEEK_CUR");
  }
  if (offset < 0) {
    throw NotSupportedException(__func__, "cannot seek backwards");
  }
  if (offset > 0) {
    int64_t avail = bufferedLen();
    assert(avail >= 0);
    if (avail >= offset) {
      m_readpos += offset;
      return true;
    }
    if (avail > 0) {
      m_readpos += avail;
      offset -= avail;
    }

    while (offset) {
      char tmp[1024];
      int64_t nread = offset > (int64_t)sizeof(tmp) ? (int64_t)sizeof(tmp) : offset;
      nread = readImpl(tmp, nread);
      if (nread <= 0) {
        return false;
      }
      offset -= nread;
    }
  }
  return true;
}

int64_t File::tell() {
  throw NotSupportedException(__func__, "cannot tell");
}

bool File::eof() {
  throw NotSupportedException(__func__, "cannot test eof");
}

bool File::rewind() {
  throw NotSupportedException(__func__, "cannot rewind");
}

bool File::flush() {
  return true;
}

bool File::truncate(int64_t size) {
  throw NotSupportedException(__func__, "cannot truncate");
}

bool File::lock(int operation) {
  bool b = false;
  return lock(operation, b);
}

bool File::lock(int operation, bool &wouldblock /* = false */) {
  assert(m_fd >= 0);

  wouldblock = false;
  if (flock(m_fd, operation)) {
    if (errno == EWOULDBLOCK) {
      wouldblock = true;
    }
    return false;
  }
  return true;
}

const StaticString
  s_wrapper_type("wrapper_type"),
  s_stream_type("stream_type"),
  s_mode("mode"),
  s_unread_bytes("unread_bytes"),
  s_seekable("seekable"),
  s_uri("uri"),
  s_timed_out("timed_out"),
  s_blocked("blocked"),
  s_eof("eof"),
  s_wrapper_data("wrapper_data");

Array File::getMetaData() {
  ArrayInit ret(10);
  ret.set(s_wrapper_type, o_getClassName());
  ret.set(s_stream_type,  getStreamType());
  ret.set(s_mode,         String(m_mode));
  ret.set(s_unread_bytes, 0);
  ret.set(s_seekable,     seekable());
  ret.set(s_uri,          String(m_name));
  ret.set(s_timed_out,    false);
  ret.set(s_blocked,      true);
  ret.set(s_eof,          eof());
  ret.set(s_wrapper_data, getWrapperMetaData());
  return ret.create();
}

///////////////////////////////////////////////////////////////////////////////
// utility functions

String File::readLine(int64_t maxlen /* = 0 */) {
  size_t current_buf_size = 0;
  size_t total_copied = 0;
  char *ret = nullptr;
  for (;;) {
    int64_t avail = bufferedLen();
    if (avail > 0) {
      int64_t cpysz = 0;
      bool done = false;

      char *readptr = m_buffer + m_readpos;
      const char *eol;
      const char *cr;
      const char *lf;
      cr = (const char *)memchr(readptr, '\r', avail);
      lf = (const char *)memchr(readptr, '\n', avail);
      if (cr && lf != cr + 1 && !(lf && lf < cr)) {
        /* mac */
        eol = cr;
      } else if ((cr && lf && cr == lf - 1) || (lf)) {
        /* dos or unix endings */
        eol = lf;
      } else {
        eol = cr;
      }

      if (eol) {
        cpysz = eol - readptr + 1;
        done = true;
      } else {
        cpysz = avail;
      }
      if (maxlen > 0 && maxlen <= cpysz) {
        cpysz = maxlen;
        done = true;
      }

      current_buf_size += cpysz + 1;
      if (ret) {
        ret = (char *)realloc(ret, current_buf_size);
      } else {
        ret = (char *)malloc(current_buf_size);
      }
      memcpy(ret + total_copied, readptr, cpysz);

      m_position += cpysz;
      m_readpos += cpysz;
      maxlen -= cpysz;
      total_copied += cpysz;

      if (done) {
        break;
      }
    } else if (eof()) {
      break;
    } else {
      if (m_buffer == nullptr) {
        m_buffer = (char *)malloc(CHUNK_SIZE);
      }
      m_writepos = readImpl(m_buffer, CHUNK_SIZE);
      m_readpos = 0;
      if (bufferedLen() == 0) {
        break;
      }
    }
  }

  if (total_copied == 0) {
    assert(ret == nullptr);
    return String();
  }

  ret[total_copied] = '\0';
  return String(ret, total_copied, AttachString);
}

String File::readRecord(const String& delimiter, int64_t maxlen /* = 0 */) {
  if (eof() && m_writepos == m_readpos) {
    return empty_string;
  }

  if (maxlen <= 0 || maxlen > CHUNK_SIZE) {
    maxlen = CHUNK_SIZE;
  }

  int64_t avail = bufferedLen();
  if (m_buffer == nullptr) {
    m_buffer = (char *)malloc(CHUNK_SIZE * 3);
  }
  if (avail < maxlen && !eof()) {
    assert(m_writepos + maxlen - avail <= CHUNK_SIZE * 3);
    m_writepos += readImpl(m_buffer + m_writepos, maxlen - avail);
    maxlen = bufferedLen();
  }
  if (m_readpos >= CHUNK_SIZE) {
    memcpy(m_buffer, m_buffer + m_readpos, bufferedLen());
    m_writepos -= m_readpos;
    m_readpos = 0;
  }

  int64_t toread;
  const char *e;
  bool skip = false;
  if (delimiter.empty()) {
    toread = maxlen;
  } else {
    if (delimiter.size() == 1) {
      e = (const char *)memchr(m_buffer + m_readpos, delimiter.charAt(0),
                               bufferedLen());
    } else {
      int64_t pos = string_find(m_buffer + m_readpos, bufferedLen(),
                              delimiter.data(), delimiter.size(), 0, true);
      if (pos >= 0) {
        e = m_buffer + m_readpos + pos;
      } else {
        e = nullptr;
      }
    }

    if (!e) {
      toread = maxlen;
    } else {
      toread = e - m_buffer - m_readpos;
      skip = true;
    }
  }

  if (toread > maxlen && maxlen > 0) {
    toread = maxlen;
  }

  if (toread >= 0) {
    String s = String(toread, ReserveString);
    char *buf = s.bufferSlice().ptr;
    if (toread) {
      memcpy(buf, m_buffer + m_readpos, toread);
    }

    m_readpos += toread;
    if (skip) {
      m_readpos += delimiter.size();
      m_position += delimiter.size();
    }
    return s.setSize(toread);
  }

  return empty_string;
}

int64_t File::print() {
  int64_t total = 0;
  while (true) {
    char buffer[1024];
    int64_t len = readImpl(buffer, 1024);
    if (len == 0) break;
    total += len;
    g_context->write(buffer, len);
  }
  return total;
}

int64_t File::printf(const String& format, CArrRef args) {
  int len = 0;
  char *output = string_printf(format.data(), format.size(), args, &len);
  return write(String(output, len, AttachString));
}

///////////////////////////////////////////////////////////////////////////////
// csv functions

int64_t File::writeCSV(CArrRef fields, char delimiter_char /* = ',' */,
                     char enclosure_char /* = '"' */) {
  int line = 0;
  int count = fields.size();
  const char escape_char = '\\';
  StringBuffer csvline(1024);

  for (ArrayIter iter(fields); iter; ++iter) {
    String value = iter.second().toString();
    bool need_enclosure = false;
    for (int i = 0; i < value.size(); i++) {
      char ch = value.charAt(i);
      if (ch == delimiter_char || ch == enclosure_char || ch == escape_char ||
          ch == '\n' || ch == '\r' || ch == '\t' || ch == ' ') {
        need_enclosure = true;
        break;
      }
    }
    if (need_enclosure) {
      csvline.append(enclosure_char);
      const char *ch = value.data();
      const char *end = ch + value.size();
      bool escaped = false;
      while (ch < end) {
        if (*ch == escape_char) {
          escaped = true;
        } else if (!escaped && *ch == enclosure_char) {
          csvline.append(enclosure_char);
        } else {
          escaped = false;
        }
        csvline.append(*ch);
        ch++;
      }
      csvline.append(enclosure_char);
    } else {
      csvline.append(value);
    }

    if (++line != count) {
      csvline.append(delimiter_char);
    }
  }
  csvline.append('\n');

  return write(csvline.detach());
}

static const char *lookup_trailing_spaces(const char *ptr, int len) {
  if (len > 0) {
    ptr += len;
    switch (*(ptr - 1)) {
    case '\n':
      if (len > 1 && *(ptr - 2) == '\r') {
        return ptr - 2;
      }
      /* break is omitted intentionally */
    case '\r':
      return ptr - 1;
    }
  }
  return ptr;
}

Array File::readCSV(int64_t length /* = 0 */,
                    char delimiter_char /* = ',' */,
                    char enclosure_char /* = '"' */,
                    char escape_char /* = '\\' */,
                    const String* input /* = nullptr */) {
  const String& line = (input != nullptr) ? *input : readLine(length);
  if (line.empty()) {
    return null_array;
  }

  String new_line;
  const char *buf = line.data();
  int64_t buf_len = line.size();

  char *temp, *tptr, *line_end, *limit;
  const char *bptr;

  int64_t temp_len, line_end_len;
  bool first_field = true;
  int inc_len;

  /* Now into new section that parses buf for delimiter/enclosure fields */

  /* Strip trailing space from buf, saving end of line in case required
     for enclosure field */
  bptr = buf;
  tptr = (char *)lookup_trailing_spaces(buf, buf_len);
  line_end_len = buf_len - (size_t)(tptr - buf);
  line_end = limit = tptr;

  /* reserve workspace for building each individual field */
  temp_len = buf_len;
  temp = (char *)malloc(temp_len + line_end_len + 1);

  /* Initialize return array */
  Array ret;

  /* Main loop to read CSV fields */
  /* NB this routine will return a single null entry for a blank line */
  do {
    char *comp_end;
    const char *hunk_begin;

    tptr = temp;

    /* 1. Strip any leading space before an enclosure */

    inc_len = (bptr < limit);
    const char *tmp = bptr;
    while ((*tmp != delimiter_char) && isspace((int)*(unsigned char *)tmp)) {
      ++tmp;
    }
    if (*tmp == enclosure_char) {
      bptr = tmp;
    }

    if (first_field && bptr == line_end) {
      ret.append(null_variant);
      break;
    }
    first_field = false;

    /* 2. Read field, leaving bptr pointing at start of next field */
    if (inc_len != 0 && *bptr == enclosure_char) {
      int state = 0;

      bptr++;  /* move on to first character in field */
      hunk_begin = bptr;

      /* 2A. handle enclosure delimited field */

      for (;;) {
        switch (inc_len) {
        case 0:
          switch (state) {
          case 2:
            memcpy(tptr, hunk_begin, bptr - hunk_begin - 1);
            tptr += (bptr - hunk_begin - 1);
            hunk_begin = bptr;
            goto quit_loop_2;

          case 1:
            memcpy(tptr, hunk_begin, bptr - hunk_begin);
            tptr += (bptr - hunk_begin);
            hunk_begin = bptr;
            /* break is omitted intentionally */
          case 0:
            {
              if (hunk_begin != line_end) {
                memcpy(tptr, hunk_begin, bptr - hunk_begin);
                tptr += (bptr - hunk_begin);
                hunk_begin = bptr;
              }
              /* add the embedded line end to the field */
              memcpy(tptr, line_end, line_end_len);
              tptr += line_end_len;

              new_line = (input != nullptr) ? String() : readLine(length);
              const char *new_buf = new_line.data();
              int64_t new_len = new_line.size();
              if (new_len == 0) {
                /* we've got an unterminated enclosure,
                 * assign all the data from the start of
                 * the enclosure to end of data to the
                 * last element */
                if ((size_t)temp_len > (size_t)(limit - buf)) {
                  goto quit_loop_2;
                }
                goto out;
              }
              temp_len += new_len;
              char *new_temp = (char*)realloc(temp, temp_len);
              tptr = new_temp + (size_t)(tptr - temp);
              temp = new_temp;

              buf_len = new_len;
              bptr = buf = new_buf;
              hunk_begin = buf;

              line_end = limit = (char *)lookup_trailing_spaces(buf, buf_len);
              line_end_len = buf_len - (size_t)(limit - buf);
              state = 0;
            }
            break;
          }
          break;
        case 1:
          /* we need to determine if the enclosure is
           * 'real' or is it escaped */
          switch (state) {
          case 1: /* escaped */
            bptr++;
            state = 0;
            break;
          case 2: /* embedded enclosure ? let's check it */
            if (*bptr != enclosure_char) {
              /* real enclosure */
              memcpy(tptr, hunk_begin, bptr - hunk_begin - 1);
              tptr += (bptr - hunk_begin - 1);
              hunk_begin = bptr;
              goto quit_loop_2;
            }
            memcpy(tptr, hunk_begin, bptr - hunk_begin);
            tptr += (bptr - hunk_begin);
            bptr++;
            hunk_begin = bptr;
            state = 0;
            break;
          default:
            if (*bptr == enclosure_char) {
              state = 2;
            } else if (*bptr == escape_char) {
              state = 1;
            }
            bptr++;
            break;
          }
          break;
        }
        inc_len = (bptr < limit ? 1 : 0);
      }

    quit_loop_2:
      /* look up for a delimiter */

      for (;;) {
        switch (inc_len) {
        case 0:
          goto quit_loop_3;

        case 1:
          if (*bptr == delimiter_char) {
            goto quit_loop_3;
          }
          break;
        default:
          break;
        }
        bptr += inc_len;
        inc_len = (bptr < limit ? 1 : 0);
      }

    quit_loop_3:
      memcpy(tptr, hunk_begin, bptr - hunk_begin);
      tptr += (bptr - hunk_begin);
      bptr += inc_len;
      comp_end = tptr;
    } else {
      /* 3B. Handle non-enclosure field */

      hunk_begin = bptr;

      for (;;) {
        switch (inc_len) {
        case 0:
          goto quit_loop_4;
        case 1:
          if (*bptr == delimiter_char) {
            goto quit_loop_4;
          }
          break;
        default:
          break;
        }
        bptr += inc_len;
        inc_len = (bptr < limit ? 1 : 0);
      }

    quit_loop_4:
      memcpy(tptr, hunk_begin, bptr - hunk_begin);
      tptr += (bptr - hunk_begin);

      comp_end = (char *)lookup_trailing_spaces(temp, tptr - temp);
      if (*bptr == delimiter_char) {
        bptr++;
      }
    }

    /* 3. Now pass our field back to php */
    *comp_end = '\0';
    ret.append(String(temp, comp_end - temp, CopyString));
  } while (inc_len > 0);
out:

  free(temp);
  return ret;
}

String File::getLastError() {
  return String(folly::errnoStr(errno).toStdString());
}


///////////////////////////////////////////////////////////////////////////////
}
