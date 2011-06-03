/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/file/file.h>
#include <runtime/base/file/plain_file.h>
#include <runtime/base/file/temp_file.h>
#include <runtime/base/file/output_file.h>
#include <runtime/base/file/zip_file.h>
#include <runtime/base/file/mem_file.h>
#include <runtime/base/file/url_file.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/util/string_buffer.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/server/static_content_cache.h>
#include <runtime/base/server/virtual_host.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/runtime_error.h>
#include <util/logger.h>
#include <util/process.h>
#include <util/util.h>
#include <runtime/base/zend/zend_string.h>
#include <runtime/base/zend/zend_printf.h>
#include <runtime/base/util/exceptions.h>
#include <sys/file.h>
#include <runtime/base/string_util.h>
#include <runtime/base/array/array_iterator.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// statics

StaticString File::s_class_name("File");
StaticString File::s_resource_name("stream");

IMPLEMENT_REQUEST_LOCAL(FileData, s_file_data);

String File::TranslatePath(CStrRef filename, bool useFileCache /* = false */,
                           bool keepRelative /*= false */) {
  String canonicalized(Util::canonicalize(string(filename.data(),
                                                 filename.size())));

  if (useFileCache) {
    String translated = TranslatePath(canonicalized, false);
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

  if (RuntimeOption::SafeFileAccess) {
    const vector<string> &allowedDirectories =
      VirtualHost::GetAllowedDirectories();
    for (unsigned int i = 0; i < allowedDirectories.size();
         i++) {
      const string &directory = allowedDirectories[i];
      int len = directory.size();
      if (canonicalized.length() >= len &&
          strncmp(canonicalized.data(), directory.data(), len) == 0) {
        return canonicalized;
      }
    }

    // disallow access with an absolute path
    if (canonicalized.charAt(0) == '/') {
      return "";
    }

    // unresolvable paths are all considered as unsafe
    if (canonicalized.find("..") >= 0) {
      ASSERT(canonicalized.find("..") == 0);
      return "";
    }
  }

  if (canonicalized.charAt(0) == '/' || keepRelative) {
    return canonicalized;
  }

  String cwd = g_context->getCwd();
  if (!cwd.empty() && cwd[cwd.length() - 1] == '/') {
    return cwd + canonicalized;
  }
  return cwd + "/" + canonicalized;
}

String File::TranslateCommand(CStrRef cmd) {
  //TODO: security checking
  return cmd;
}

bool File::IsVirtualDirectory(CStrRef filename) {
  if (StaticContentCache::TheFileCache &&
      StaticContentCache::TheFileCache->dirExists(filename.data(), false)) {
    return true;
  }
  return false;
}

bool File::IsPlainFilePath(CStrRef filename) {
  return filename.find("://") == String::npos;
}

Variant File::Open(CStrRef filename, CStrRef mode,
                   CArrRef options /* = null_array */) {
  Object ret = OpenImpl(filename, mode, options);
  if (!ret.isNull()) {
    File *file = ret.getTyped<File>();
    file->m_name = filename.data();
    file->m_mode = mode.data();
    return ret;
  }
  return false;
}

Object File::OpenImpl(CStrRef filename, CStrRef mode, CArrRef options) {
  static const char http_prefix[] = "http://";
  static const char https_prefix[] = "https://";
  static const char zlib_prefix[] = "compress.zlib://";

  if (!strncasecmp(filename.c_str(), "php://", 6)) {
    if (!strcasecmp(filename.c_str(), "php://stdin")) {
      return Object(NEWOBJ(PlainFile)(dup(STDIN_FILENO), true));
    }
    if (!strcasecmp(filename.c_str(), "php://stdout")) {
      return Object(NEWOBJ(PlainFile)(dup(STDOUT_FILENO), true));
    }
    if (!strcasecmp(filename.c_str(), "php://stderr")) {
      return Object(NEWOBJ(PlainFile)(dup(STDERR_FILENO), true));
    }

    if (!strncasecmp(filename.c_str(), "php://temp", 10) ||
        !strcasecmp(filename.c_str(), "php://memory")) {
      TempFile *file = NEWOBJ(TempFile)();
      if (!file->valid()) {
        raise_warning("Unable to create temporary file");
        return Object();
      }
      return Object(file);
    }

    if (!strcasecmp(filename.c_str(), "php://input")) {
      Transport *transport = g_context->getTransport();
      if (transport) {
        int size = 0;
        const void *data = transport->getPostData(size);
        if (data && size) {
          return Object(NEWOBJ(MemFile)((const char *)data, size));
        }
      }
      return Object(NEWOBJ(MemFile)(NULL, 0));
    }

    if (!strcasecmp(filename.c_str(), "php://output")) {
      return Object(NEWOBJ(OutputFile)(filename));
    }

    raise_warning("Unable to open file %s", filename.c_str());
    return Object();
  }

  if (!strncmp(filename.data(), http_prefix, sizeof(http_prefix) - 1) ||
      !strncmp(filename.data(), https_prefix, sizeof(https_prefix) - 1)) {
    UrlFile *file;
    if (options.isNull()) {
      file = NEWOBJ(UrlFile)();
    } else {
      Array opts = options["http"];
      String method = "GET";
      if (opts.exists("method")) {
        method = opts["method"].toString();
      }
      Array headers;
      if (opts.exists("header")) {
        Array lines = StringUtil::Explode(opts["header"].toString(), "\r\n");
        for (ArrayIter it(lines); it; ++it) {
          Array parts = StringUtil::Explode(it.second().toString(), ": ");
          headers.set(parts.rvalAt(0), parts.rvalAt(1));
        }
        if (opts.exists("user_agent") && !headers.exists("User-Agent")) {
          headers.set("User_Agent", opts["user_agent"]);
        }
      }
      int max_redirs = 20;
      if (opts.exists("max_redirects")) max_redirs = opts["max_redirects"];
      int timeout = -1;
      if (opts.exists("timeout")) timeout = opts["timeout"];
      file = NEWOBJ(UrlFile)(method.data(), headers, opts["content"].toString(),
                          max_redirs, timeout);
    }
    Object obj(file);
    bool ret = file->open(filename, mode);
    if (!ret) {
      raise_warning("Failed to open %s (%s)", filename.data(),
                    file->getLastError().c_str());
      return Object();
    }
    return obj;
  }

  bool gzipped = false;
  String name = filename;
  if (!strncmp(filename.data(), zlib_prefix, sizeof(zlib_prefix) - 1)) {
    name = filename.substr(sizeof(zlib_prefix) - 1);
    gzipped = true;
  }

  // try to read from the file cache first
  if (StaticContentCache::TheFileCache) {
    string relative =
      FileCache::GetRelativePath(File::TranslatePath(name).c_str());
    MemFile *file = NEWOBJ(MemFile)();
    Object obj(file);
    bool ret = file->open(relative, mode);
    if (ret) {
      if (gzipped) {
        file->unzip();
      }
      return obj;
    }
  }

  if (gzipped) {
    ZipFile *file = NEWOBJ(ZipFile)();
    Object obj(file);
    bool ret = file->open(File::TranslatePath(name), mode);
    if (!ret) {
      raise_warning("%s", file->getLastError().c_str());
      return Object();
    }
    return obj;
  }

  PlainFile *file = NEWOBJ(PlainFile)();
  Object obj(file);
  bool ret = file->open(File::TranslatePath(name), mode);
  if (!ret) {
    raise_warning("%s", file->getLastError().c_str());
    return Object();
  }
  return obj;
}

///////////////////////////////////////////////////////////////////////////////
// constructor and destructor

File::File(bool nonblocking)
  : m_fd(-1), m_closed(false), m_nonblocking(nonblocking), m_writepos(0),
    m_readpos(0), m_position(0), m_buffer(NULL) {
}

File::~File() {
  closeImpl();
}

void File::closeImpl() {
  if (m_buffer) {
    free(m_buffer);
    m_buffer = NULL;
  }
}

///////////////////////////////////////////////////////////////////////////////
// default implementation of virtual functions

int File::getc() {
  if (m_writepos > m_readpos) {
    m_position++;
    return m_buffer[m_readpos++] & 0xff;
  }

  char buffer[1];
  int64 len = readImpl(buffer, 1);
  if (len != 1) {
    return EOF;
  }
  m_position += len;
  return (int)(unsigned char)buffer[0];
}

String File::read(int64 length) {
  if (length <= 0) {
    throw_invalid_argument("Invalid length %d", length);
    return "";
  }

  char *ret = (char *)malloc(length + 1);
  int64 copied = 0;
  int64 avail = m_writepos - m_readpos;

  while (avail < length && !eof()) {
    if (m_buffer == NULL) {
      m_buffer = (char *)malloc(CHUNK_SIZE);
    }

    if (avail > 0) {
      memcpy(ret + copied, m_buffer + m_readpos, avail);
      copied += avail;
      length -= avail;
    }

    m_writepos = readImpl(m_buffer, CHUNK_SIZE);
    m_readpos = 0;
    avail = m_writepos - m_readpos;

    if (avail == 0 || m_nonblocking) {
      // For nonblocking mode, temporary out of data.
      break;
    }
  }

  avail = m_writepos - m_readpos;
  if (avail > 0) {
    int64 n = length < avail ? length : avail;
    memcpy(ret + copied, m_buffer + m_readpos, n);
    m_readpos += n;
    copied += n;
  }

  m_position += copied;
  ret[copied] = '\0';
  return String(ret, copied, AttachString);
}

int64 File::write(CStrRef data, int64 length /* = 0 */) {
  if (seekable()) {
    m_readpos = m_writepos = 0; // invalidating read buffer
    seek(m_position, SEEK_SET);
  }
  if (length <= 0 || length > data.size()) {
    length = data.size();
  }
  if (length) {
    int64 written = writeImpl(data.data(), length);
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

bool File::seek(int64 offset, int whence /* = SEEK_SET */) {
  if (whence != SEEK_CUR) {
    throw NotSupportedException(__func__, "cannot seek other than SEEK_CUR");
  }
  if (offset < 0) {
    throw NotSupportedException(__func__, "cannot seek backwards");
  }
  if (offset > 0) {
    int64 avail = m_writepos - m_readpos;
    ASSERT(avail >= 0);
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
      int64 nread = offset > (int64)sizeof(tmp) ? (int64)sizeof(tmp) : offset;
      nread = readImpl(tmp, nread);
      if (nread <= 0) {
        return false;
      }
      offset -= nread;
    }
  }
  return true;
}

int64 File::tell() {
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

bool File::truncate(int64 size) {
  throw NotSupportedException(__func__, "cannot truncate");
}

bool File::lock(int operation) {
  bool b = false;
  return lock(operation, b);
}

bool File::lock(int operation, bool &wouldblock /* = false */) {
  ASSERT(m_fd >= 0);

  wouldblock = false;
  if (flock(m_fd, operation)) {
    if (errno == EWOULDBLOCK) {
      wouldblock = true;
    }
    return false;
  }
  return true;
}

Array File::getMetaData() {
  Array ret = Array::Create();
  ret.set("wrapper_type", o_getClassName());
  ret.set("stream_type",  getStreamType());
  ret.set("mode",         String(m_mode));
  ret.set("unread_bytes", 0);
  ret.set("seekable",     seekable());
  ret.set("uri",          String(m_name));
  ret.set("timed_out",    false);
  ret.set("blocked",      true);
  ret.set("eof",          eof());
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// utility functions

String File::readLine(int64 maxlen /* = 0 */) {
  size_t current_buf_size = 0;
  size_t total_copied = 0;
  char *ret = NULL;
  for (;;) {
    int64 avail = m_writepos - m_readpos;
    if (avail > 0) {
      int64 cpysz = 0;
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
      if (m_buffer == NULL) {
        m_buffer = (char *)malloc(CHUNK_SIZE);
      }
      m_writepos = readImpl(m_buffer, CHUNK_SIZE);
      m_readpos = 0;
      if (m_writepos - m_readpos == 0) {
        break;
      }
    }
  }

  if (total_copied == 0) {
    ASSERT(ret == NULL);
    return String();
  }

  ret[total_copied] = '\0';
  return String(ret, total_copied, AttachString);
}

String File::readRecord(CStrRef delimiter, int64 maxlen /* = 0 */) {
  if (eof() && m_writepos == m_readpos) {
    return empty_string;
  }

  if (maxlen <= 0 || maxlen > CHUNK_SIZE) {
    maxlen = CHUNK_SIZE;
  }

  int64 avail = m_writepos - m_readpos;
  if (m_buffer == NULL) {
    m_buffer = (char *)malloc(CHUNK_SIZE * 3);
  }
  if (avail < maxlen && !eof()) {
    ASSERT(m_writepos + maxlen - avail <= CHUNK_SIZE * 3);
    m_writepos += readImpl(m_buffer + m_writepos, maxlen - avail);
    maxlen = m_writepos - m_readpos;
  }
  if (m_readpos >= CHUNK_SIZE) {
    memcpy(m_buffer, m_buffer + m_readpos, m_writepos - m_readpos);
    m_writepos -= m_readpos;
    m_readpos = 0;
  }

  int64 toread;
  const char *e;
  bool skip = false;
  if (delimiter.empty()) {
    toread = maxlen;
  } else {
    if (delimiter.size() == 1) {
      e = (const char *)memchr(m_buffer + m_readpos, delimiter.charAt(0),
                               m_writepos - m_readpos);
    } else {
      int64 pos = string_find(m_buffer + m_readpos, m_writepos - m_readpos,
                              delimiter.data(), delimiter.size(), 0, true);
      if (pos >= 0) {
        e = m_buffer + m_readpos + pos;
      } else {
        e = NULL;
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
    char *buf = (char *)malloc(toread + 1);
    if (toread) {
      memcpy(buf, m_buffer + m_readpos, toread);
    }

    m_readpos += toread;
    if (skip) {
      m_readpos += delimiter.size();
      m_position += delimiter.size();
    }
    buf[toread] = '\0';
    return String(buf, toread, AttachString);
  }

  return empty_string;
}

int64 File::print() {
  int64 total = 0;
  while (true) {
    char buffer[1024];
    int64 len = readImpl(buffer, 1024);
    if (len == 0) break;
    total += len;
    g_context->write(buffer, len);
  }
  return total;
}

int64 File::printf(CStrRef format, CArrRef args) {
  int len = 0;
  char *output = string_printf(format.data(), format.size(), args, &len);
  return write(String(output, len, AttachString));
}

///////////////////////////////////////////////////////////////////////////////
// csv functions

int64 File::writeCSV(CArrRef fields, char delimiter_char /* = ',' */,
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

Array File::readCSV(int64 length /* = 0 */, char delimiter_char /* = ',' */,
                    char enclosure_char /* = '"' */) {
  String line = readLine(length);
  if (line.empty()) {
    return Array();
  }

  String new_line;
  const char *buf = line.data();
  int64 buf_len = line.size();

  char *temp, *tptr, *line_end, *limit;
  const char *bptr;
  const char escape_char = '\\';

  int64 temp_len, line_end_len;
  bool first_field = true;

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

    /* 1. Strip any leading space */
    for (; bptr < limit; ++bptr) {
      if (!isspace((int)*(unsigned char *)bptr) || *bptr == delimiter_char) {
        break;
      }
    }

    if (first_field && bptr == line_end) {
      ret.append(null_variant);
      break;
    }
    first_field = false;

    /* 2. Read field, leaving bptr pointing at start of next field */
    if (bptr < limit && *bptr == enclosure_char) {
      int state = 0;

      bptr++;  /* move on to first character in field */
      hunk_begin = bptr;

      /* 2A. handle enclosure delimited field */

      int inc_len = 1;
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

              new_line = readLine(length);
              const char *new_buf = new_line.data();
              int64 new_len = new_line.size();
              if (new_len == 0) {
                /* we've got an unterminated enclosure,
                 * assign all the data from the start of
                 * the enclosure to end of data to the
                 * last element */
                if ((size_t)temp_len > (size_t)(limit - buf)) {
                  goto quit_loop_2;
                }
                return ret;
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
            if (*bptr == escape_char) {
              state = 1;
            } else if (*bptr == enclosure_char) {
              state = 2;
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
      for (; bptr < limit; ++bptr) {
        if (*bptr == delimiter_char) {
          break;
        }
      }

      memcpy(tptr, hunk_begin, bptr - hunk_begin);
      tptr += (bptr - hunk_begin);
      if (bptr < limit) ++bptr;
      comp_end = tptr;
    } else {
      /* 2B. Handle non-enclosure field */

      hunk_begin = bptr;

      for (; bptr < limit; ++bptr) {
        if (*bptr == delimiter_char) {
          break;
        }
      }
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
  } while (bptr < limit);

  free(temp);
  return ret;
}

String File::getLastError() {
  return Util::safe_strerror(errno);
}


///////////////////////////////////////////////////////////////////////////////
}
