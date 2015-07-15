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

#include "hphp/runtime/base/file.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/zend-printf.h"
#include "hphp/runtime/base/zend-string.h"

#include "hphp/runtime/ext/stream/ext_stream-user-filters.h"

#include "hphp/runtime/server/static-content-cache.h"
#include "hphp/runtime/server/virtual-host.h"

#include "hphp/system/constants.h"

#include "hphp/runtime/base/file-util.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"

#include <folly/String.h>

#include <algorithm>
#include <sys/file.h>

namespace HPHP {

const int FileData::CHUNK_SIZE = 8192;

FileData::FileData(bool nonblocking)
: m_nonblocking(nonblocking)
{ }

bool FileData::closeImpl() {
  free(m_buffer);
  m_buffer = nullptr;
  return true;
}

FileData::~FileData() {
  FileData::closeImpl();
}

///////////////////////////////////////////////////////////////////////////////
// statics

StaticString File::s_resource_name("stream");

int __thread s_pcloseRet;

const int File::CHUNK_SIZE = FileData::CHUNK_SIZE;
const int File::USE_INCLUDE_PATH = 1;

String File::TranslatePathKeepRelative(const String& filename) {
  // canonicalize asserts that we don't have nulls
  String canonicalized = FileUtil::canonicalize(filename);
  if (ThreadInfo::s_threadInfo->m_reqInjectionData.hasSafeFileAccess()) {
    auto const& allowedDirectories = ThreadInfo::s_threadInfo->
      m_reqInjectionData.getAllowedDirectories();
    auto it = std::upper_bound(allowedDirectories.begin(),
                               allowedDirectories.end(), canonicalized,
                               [](const String& val, const std::string& dir) {
                                 return strcmp(val.c_str(), dir.c_str()) < 0;
                               });
    if (it != allowedDirectories.begin()) {
      const std::string& dir = *--it;
      if (dir.size() <= canonicalized.size() &&
          !strncmp(dir.c_str(), canonicalized.c_str(), dir.size())) {
        return canonicalized;
      }
    }

    // disallow access with an absolute path
    if (canonicalized.charAt(0) == '/') {
      return empty_string();
    }

    // unresolvable paths are all considered as unsafe
    if (canonicalized.find("..") >= 0) {
      assert(canonicalized.find("..") == 0);
      return empty_string();
    }
  }

  return canonicalized;
}

String File::TranslatePath(const String& filename) {
  if (filename.charAt(0) != '/') {
    String cwd = g_context->getCwd();
    return TranslatePathKeepRelative(cwd + "/" + filename);
  }
  return TranslatePathKeepRelative(filename);
}

String File::TranslatePathWithFileCache(const String& filename) {
  // canonicalize asserts that we don't have nulls
  String canonicalized = FileUtil::canonicalize(filename);
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
  return
    StaticContentCache::TheFileCache &&
    StaticContentCache::TheFileCache->dirExists(filename.data(), false);
}

bool File::IsVirtualFile(const String& filename) {
  return
    StaticContentCache::TheFileCache &&
    StaticContentCache::TheFileCache->fileExists(filename.data(), false);
}

req::ptr<File> File::Open(const String& filename, const String& mode,
                          int options /* = 0 */,
                          const req::ptr<StreamContext>& context /* = null */) {
  Stream::Wrapper *wrapper = Stream::getWrapperFromURI(filename);
  if (!wrapper) return nullptr;
  if (filename.find('\0') >= 0) return nullptr;
  auto rcontext = context ? context : g_context->getStreamContext();
  auto file = wrapper->open(filename, mode, options, rcontext);
  if (file) {
    file->m_data->m_name = filename.data();
    file->m_data->m_mode = mode.data();
    file->m_streamContext = rcontext;
  }
  return file;
}

///////////////////////////////////////////////////////////////////////////////
// constructor and destructor

File::File(
  std::shared_ptr<FileData> data,
  const String& wrapper_type, /* = null_string */
  const String& stream_type /* = empty_string_ref*/)
: m_data(data),
  m_wrapperType(wrapper_type.get()),
  m_streamType(stream_type.get())
{ }

File::File(bool nonblocking /* = true */,
           const String& wrapper_type /* = null_string */,
           const String& stream_type /* = empty_string_ref */)
: File(std::make_shared<FileData>(nonblocking), wrapper_type, stream_type)
{ }

File::~File() {
  if(m_data.unique()) {
    closeImpl();
  }
  m_data.reset();
}

void File::sweep() {
  // Clear non-request-local state without deleting `this`. Therefore assumes
  // `this` has been request-heap allocated. Note that the derived class'
  // sweep() is responsible for closing m_fd and any other non-request
  // resources it might have allocated.
  assert(!valid());
  File::closeImpl();
  m_data.reset();
  m_wrapperType = nullptr;
  m_streamType = nullptr;
}

bool File::closeImpl() {
  return m_data ? m_data->closeImpl() : true;
}

void File::invokeFiltersOnClose() {
  if (MemoryManager::sweeping()) {
    return;
  }
  // As it's being closed, we can't actually do anything with filter output
  applyFilters(
    empty_string_ref,
    m_readFilters,
    /* closing = */ true
  );
  if (!m_writeFilters.empty()) {
    auto buf = applyFilters(
      empty_string_ref,
      m_writeFilters,
      /* closing = */ true
    );
    if (buf.length() > 0) {
      writeImpl(buf.data(), buf.length());
    }
  }
  for (const auto& filter: m_readFilters) {
    filter->invokeOnClose();
  }
  for (const auto& filter: m_writeFilters) {
    filter->invokeOnClose();
  }
}

///////////////////////////////////////////////////////////////////////////////
// default implementation of virtual functions

int File::getc() {
  if (m_data->m_writepos > m_data->m_readpos) {
    m_data->m_position++;
    return m_data->m_buffer[m_data->m_readpos++] & 0xff;
  }

  char buffer[1];
  int64_t len = readImpl(buffer, 1);
  if (len != 1) {
    return EOF;
  }
  m_data->m_position += len;
  return (int)(unsigned char)buffer[0];
}

String File::read() {
  StringBuffer sb;
  int64_t copied = 0;
  int64_t avail = bufferedLen();

  while (!eof() || avail) {
    if (m_data->m_buffer == nullptr) {
      m_data->m_buffer = (char *)malloc(CHUNK_SIZE);
      m_data->m_bufferSize = CHUNK_SIZE;
    }

    if (avail > 0) {
      sb.append(m_data->m_buffer + m_data->m_readpos, avail);
      copied += avail;
    }

    m_data->m_writepos = filteredReadToBuffer();
    m_data->m_readpos = 0;
    avail = bufferedLen();

    if (avail == 0) {
      break;
    }
  }

  m_data->m_position += copied;
  return sb.detach();
}

String File::read(int64_t length) {
  if (length <= 0) {
    raise_notice("Invalid length %" PRId64, length);
    // XXX: Changing this to empty_string causes problems, something is
    // writing to this upstream but I'm not sure what and since it's
    // unlikely to provide significant gain alone I'm leaving it for now.
    return "";
  }

  auto const allocSize = length;
  String s = String(allocSize, ReserveString);
  char *ret = s.mutableData();
  int64_t copied = 0;
  int64_t avail = bufferedLen();

  while (avail < length && !eof()) {
    if (m_data->m_buffer == nullptr) {
      m_data->m_buffer = (char *)malloc(CHUNK_SIZE);
      m_data->m_bufferSize = CHUNK_SIZE;
    }

    if (avail > 0) {
      memcpy(ret + copied, m_data->m_buffer + m_data->m_readpos, avail);
      copied += avail;
      length -= avail;
    }

    m_data->m_writepos = filteredReadToBuffer();
    m_data->m_readpos = 0;
    avail = bufferedLen();

    if (avail == 0 || m_data->m_nonblocking) {
      // For nonblocking mode, temporary out of data.
      break;
    }
  }

  avail = bufferedLen();
  if (avail > 0) {
    int64_t n = length < avail ? length : avail;
    memcpy(ret + copied, m_data->m_buffer + m_data->m_readpos, n);
    m_data->m_readpos += n;
    copied += n;
  }

  m_data->m_position += copied;

  assert(copied <= allocSize);
  s.shrink(copied);
  return s;
}

int64_t File::filteredReadToBuffer() {
  int64_t bytes_read = readImpl(m_data->m_buffer, CHUNK_SIZE);
  if (LIKELY(m_readFilters.empty())) {
    return bytes_read;
  }

  String data(m_data->m_buffer, bytes_read, CopyString);
  String filtered = applyFilters(data,
                                 m_readFilters,
                                 /* closing = */ false);
  if (filtered.length() > m_data->m_bufferSize) {
    auto new_buffer = realloc(m_data->m_buffer, filtered.length());
    if (!new_buffer) {
      raise_error("Failed to realloc buffer");
      return 0;
    }
    m_data->m_buffer = (char*) new_buffer;
    m_data->m_bufferSize = filtered.length();
  }
  memcpy(m_data->m_buffer, filtered.data(), filtered.length());
  return filtered.length();
}

int64_t File::filteredWrite(const char* buffer, int64_t length) {
  if (LIKELY(m_writeFilters.empty())) {
    return writeImpl(buffer, length);
  }

  String data(buffer, length, CopyString);
  String filtered = applyFilters(data,
                                 m_writeFilters,
                                 /* closing = */ false);

  if (!filtered.empty()) {
    int64_t written = writeImpl(filtered.data(), filtered.size());
    m_data->m_position += written;
  }
  return 0;
}

int64_t File::write(const String& data, int64_t length /* = 0 */) {
  if (seekable()) {
    int64_t offset = m_data->m_readpos - m_data->m_writepos;
    // Writing shouldn't change the EOF status, but because we have a
    // transparent buffer, we need to do read operations on the backing
    // store, which can.
    //
    // EOF state isn't just a matter of position on all subclasses;
    // even seek(0, SEEK_CUR) can change it.
    auto eof = m_data->m_eof;
    m_data->m_readpos = m_data->m_writepos = 0; // invalidating read buffer
    seek(offset, SEEK_CUR);
    m_data->m_eof = eof;
  }

  if (length <= 0 || length > data.size()) {
    length = data.size();
  }

  if (!length) {
    return 0;
  }

  int64_t written = filteredWrite(data.data(), length);
  m_data->m_position += written;
  return written;
}

int File::putc(char c) {
  char buf[1];
  buf[0] = c;
  int ret = filteredWrite(buf, 1);
  m_data->m_position += ret;
  return ret;
}

bool File::seek(int64_t offset, int whence /* = SEEK_SET */) {
  if (whence != SEEK_CUR) {
    throw_not_supported(__func__, "cannot seek other than SEEK_CUR");
  }
  if (offset < 0) {
    throw_not_supported(__func__, "cannot seek backwards");
  }
  if (offset > 0) {
    int64_t avail = bufferedLen();
    assert(avail >= 0);
    if (avail >= offset) {
      m_data->m_readpos += offset;
      return true;
    }
    if (avail > 0) {
      m_data->m_readpos += avail;
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
  throw_not_supported(__func__, "cannot tell");
}

bool File::eof() {
  throw_not_supported(__func__, "cannot test eof");
}

bool File::rewind() {
  throw_not_supported(__func__, "cannot rewind");
}

bool File::flush() {
  return true;
}

bool File::truncate(int64_t size) {
  throw_not_supported(__func__, "cannot truncate");
}

bool File::lock(int operation) {
  bool b = false;
  return lock(operation, b);
}

bool File::lock(int operation, bool &wouldblock /* = false */) {
  assert(m_data->m_fd >= 0);

  wouldblock = false;
  if (flock(m_data->m_fd, operation)) {
    if (errno == EWOULDBLOCK) {
      wouldblock = true;
    }
    return false;
  }
  return true;
}

bool File::stat(struct stat *sb) {
  // Undocumented, but Zend returns false for streams where fstat is unsupported
  return false;
}

void File::appendReadFilter(const req::ptr<StreamFilter>& filter) {
  m_readFilters.push_back(filter);
}

void File::appendWriteFilter(const req::ptr<StreamFilter>& filter) {
  m_writeFilters.push_back(filter);
}

void File::prependReadFilter(const req::ptr<StreamFilter>& filter) {
  m_readFilters.push_front(filter);
}

void File::prependWriteFilter(const req::ptr<StreamFilter>& filter) {
  m_writeFilters.push_front(filter);
}

bool File::removeFilter(const req::ptr<StreamFilter>& filter) {
  for (auto it = m_readFilters.begin(); it != m_readFilters.end(); ++it) {
    if (*it == filter) {
      m_readFilters.erase(it);
      return true;
    }
  }
  for (auto it = m_writeFilters.begin(); it != m_writeFilters.end(); ++it) {
    if (*it == filter) {
      std::list<req::ptr<StreamFilter>> closing_filters;
      closing_filters.push_back(filter);
      String result(applyFilters(empty_string_ref,
                                 closing_filters,
                                 /* closing = */ true));
      std::list<req::ptr<StreamFilter>> later_filters;
      auto dupit(it);
      for (++dupit; dupit != m_writeFilters.end(); ++dupit) {
        later_filters.push_back(*dupit);
      }
      result = applyFilters(result, later_filters, false);
      if (!result.empty()) {
        int64_t written = writeImpl(result.data(), result.size());
        m_data->m_position += written;
      }
      m_writeFilters.erase(it);
      return true;
    }
  }
  return false;
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
  return make_map_array(
    s_wrapper_type, getWrapperType(),
    s_stream_type,  getStreamType(),
    s_mode,         String(m_data->m_mode),
    s_unread_bytes, 0,
    s_seekable,     seekable(),
    s_uri,          String(m_data->m_name),
    s_timed_out,    false,
    s_blocked,      true,
    s_eof,          eof(),
    s_wrapper_data, getWrapperMetaData()
  );
}

String File::getWrapperType() const {
  if (!m_wrapperType || m_wrapperType->empty()) {
    return o_getClassName();
  }
  return m_wrapperType;
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

      char *readptr = m_data->m_buffer + m_data->m_readpos;
      const char *eol = nullptr;
      const char *cr;
      const char *lf;
      cr = (const char *)memchr(readptr, '\r', avail);
      lf = (const char *)memchr(readptr, '\n', avail);
      if (cr && lf != cr + 1 && !(lf && lf < cr) && cr != &readptr[avail - 1]) {
        /* mac */
        eol = cr;
      } else if ((cr && lf && cr == lf - 1) || (lf)) {
        /* dos or unix endings */
        eol = lf;
      } else if (cr != &readptr[avail - 1]) {
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

      m_data->m_position += cpysz;
      m_data->m_readpos += cpysz;
      maxlen -= cpysz;
      total_copied += cpysz;

      if (done) {
        break;
      }
    } else if (eof()) {
      break;
    } else {
      if (m_data->m_buffer == nullptr) {
        m_data->m_buffer = (char *)malloc(CHUNK_SIZE);
        m_data->m_bufferSize = CHUNK_SIZE;
      }
      m_data->m_writepos = filteredReadToBuffer();
      m_data->m_readpos = 0;
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

Variant File::readRecord(const String& delimiter, int64_t maxlen /* = 0 */) {
  if (eof() && m_data->m_writepos == m_data->m_readpos) {
    return false;
  }

  if (maxlen <= 0 || maxlen > CHUNK_SIZE) {
    maxlen = CHUNK_SIZE;
  }

  int64_t avail = bufferedLen();
  if (m_data->m_buffer == nullptr) {
    m_data->m_buffer = (char *)malloc(CHUNK_SIZE * 3);
  }
  if (avail < maxlen && !eof()) {
    assert(m_data->m_writepos + maxlen - avail <= CHUNK_SIZE * 3);
    m_data->m_writepos +=
      readImpl(m_data->m_buffer + m_data->m_writepos, maxlen - avail);
    maxlen = bufferedLen();
  }
  if (m_data->m_readpos >= CHUNK_SIZE) {
    memcpy(m_data->m_buffer,
           m_data->m_buffer + m_data->m_readpos,
           bufferedLen());
    m_data->m_writepos -= m_data->m_readpos;
    m_data->m_readpos = 0;
  }

  int64_t toread;
  const char *e;
  bool skip = false;
  if (delimiter.empty()) {
    toread = maxlen;
  } else {
    if (delimiter.size() == 1) {
      e = (const char *)memchr(m_data->m_buffer + m_data->m_readpos,
                               delimiter.charAt(0),
                               bufferedLen());
    } else {
      int64_t pos = string_find(m_data->m_buffer + m_data->m_readpos,
                                bufferedLen(),
                                delimiter.data(),
                                delimiter.size(),
                                0,
                                true);
      if (pos >= 0) {
        e = m_data->m_buffer + m_data->m_readpos + pos;
      } else {
        e = nullptr;
      }
    }

    if (!e) {
      toread = maxlen;
    } else {
      toread = e - m_data->m_buffer - m_data->m_readpos;
      skip = true;
    }
  }

  if (toread > maxlen && maxlen > 0) {
    toread = maxlen;
  }

  if (toread >= 0) {
    String s = String(toread, ReserveString);
    char *buf = s.mutableData();
    if (toread) {
      memcpy(buf, m_data->m_buffer + m_data->m_readpos, toread);
    }

    m_data->m_readpos += toread;
    if (skip) {
      m_data->m_readpos += delimiter.size();
      m_data->m_position += delimiter.size();
    }
    s.setSize(toread);
    return s;
  }

  return empty_string();
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

int64_t File::printf(const String& format, const Array& args) {
  String str = string_printf(format.data(), format.size(), args);
  return write(str);
}

const StaticString s_Unknown("Unknown");
const String& File::o_getResourceName() const {
  if (isInvalid()) return s_Unknown;
  return s_resource_name;
}

///////////////////////////////////////////////////////////////////////////////
// csv functions

int64_t File::writeCSV(const Array& fields, char delimiter_char /* = ',' */,
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
    return Array();
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

template<class ResourceList>
String File::applyFilters(const String& buffer,
                          ResourceList& filters,
                          bool closing) {
  if (buffer.empty() && !closing) {
    return buffer;
  }
  req::ptr<BucketBrigade> in;
  req::ptr<BucketBrigade> out;

  if (buffer.empty()) {
    out = req::make<BucketBrigade>();
  } else {
    out = req::make<BucketBrigade>(buffer);
  }

  for (const auto& filter : filters) {
    in = out;
    out = req::make<BucketBrigade>();

    auto result = filter->invokeFilter(in, out, closing);
    // PSFS_ERR_FATAL doesn't raise a fatal in Zend - appears to be
    // treated the same as PSFS_FEED_ME
    if (UNLIKELY(result != k_PSFS_PASS_ON)) {
      return empty_string();
    }
  }

  assert(out);
  return out->createString();
}

///////////////////////////////////////////////////////////////////////////////
}
