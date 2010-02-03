/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <cpp/base/file/mem_file.h>
#include <cpp/base/type_string.h>
#include <cpp/base/util/http_client.h>
#include <cpp/base/server/static_content_cache.h>
#include <util/compression.h>

using namespace std;

namespace HPHP {

IMPLEMENT_OBJECT_ALLOCATION(MemFile);
///////////////////////////////////////////////////////////////////////////////

MemFile::MemFile()
  : m_data(NULL), m_len(-1), m_cursor(0), m_malloced(false) {
}

MemFile::MemFile(const char *data, int len)
  : m_data(NULL), m_len(len), m_cursor(0), m_malloced(true) {
  m_data = (char*)malloc(len + 1);
  if (m_data && len) {
    memcpy(m_data, data, len);
  }
  m_data[len] = '\0';
}

MemFile::~MemFile() {
  closeImpl();
}

bool MemFile::open(CStrRef filename, CStrRef mode) {
  ASSERT(m_len == -1);
  // mem files are read-only
  if (strchr(mode, '+') || strchr(mode, 'a') || strchr(mode, 'w')) {
    return false;
  }
  int len = -1;
  bool compressed = false;
  char *data =
    StaticContentCache::TheFileCache->read(filename.c_str(), len, compressed);
  if (len != -1) {
    m_name = filename;
    m_data = data;
    m_len = len;
    return true;
  }
  return false;
}

bool MemFile::close() {
  return closeImpl();
}

bool MemFile::closeImpl() {
  m_closed = true;
  if (m_malloced && m_data) {
    free(m_data);
    m_data = NULL;
  }
  File::closeImpl();
  return true;
}

///////////////////////////////////////////////////////////////////////////////

int MemFile::readImpl(char *buffer, int length) {
  ASSERT(m_len != -1);
  ASSERT(length > 0);
  int remaining = m_len - m_cursor;
  if (remaining < length) length = remaining;
  if (length > 0) {
    memcpy(buffer, (const void *)(m_data + m_cursor), length);
  }
  m_cursor += length;
  return length;
}

int MemFile::getc() {
  ASSERT(m_len != -1);
  return File::getc();
}

bool MemFile::seek(int offset, int whence /* = SEEK_SET */) {
  ASSERT(m_len != -1);
  if (whence == SEEK_CUR) {
    if (offset > 0 && offset < m_writepos - m_readpos) {
      m_readpos += offset;
      m_position += offset;
      return true;
    }
    offset += m_position;
    whence = SEEK_SET;
  }

  // invalidate the current buffer
  m_writepos = 0;
  m_readpos = 0;
  if (whence == SEEK_SET) {
    m_cursor = offset;
  } else {
    ASSERT(whence == SEEK_END);
    m_cursor = m_len + offset;
  }
  m_position = m_cursor;
  return true;
}

int MemFile::tell() {
  ASSERT(m_len != -1);
  return m_position;
}

bool MemFile::eof() {
  ASSERT(m_len != -1);
  int avail = m_writepos - m_readpos;
  if (avail > 0) {
    return false;
  }
  return m_cursor == m_len;
}

bool MemFile::rewind() {
  ASSERT(m_len != -1);
  m_cursor = 0;
  m_writepos = 0;
  m_readpos = 0;
  m_position = 0;
  return true;
}

int MemFile::writeImpl(const char *buffer, int length) {
  throw FatalErrorException((string("cannot write a mem stream: ") +
                             m_name).c_str());
}

bool MemFile::flush() {
  throw FatalErrorException((string("cannot flush a mem stream: ") +
                             m_name).c_str());
}

///////////////////////////////////////////////////////////////////////////////

void MemFile::unzip() {
  ASSERT(m_len != -1);
  ASSERT(!m_malloced);
  ASSERT(m_cursor == 0);
  char *data = gzdecode(m_data, m_len);
  if (data == NULL) {
    throw FatalErrorException((string("cannot unzip mem stream: ") +
                               m_name).c_str());
  }
  m_data = data;
  m_malloced = true;
}

///////////////////////////////////////////////////////////////////////////////
}
