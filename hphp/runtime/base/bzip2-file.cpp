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
#include "hphp/runtime/base/bzip2-file.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

BZ2File::BZ2File(): m_bzFile(nullptr), m_eof(false) {
  m_innerFile = NEWOBJ(PlainFile)();
  m_innerFile->unregister();
  m_isLocal = m_innerFile->m_isLocal;
}

BZ2File::BZ2File(PlainFile* innerFile): m_bzFile(nullptr), m_eof(false) {
  m_innerFile = innerFile;
  m_isLocal = m_innerFile->m_isLocal;
}

BZ2File::~BZ2File() {
  if (m_bzFile)
    closeImpl();
}

void BZ2File::sweep() {
  if (m_bzFile) {
    closeImpl();
  }
  File::sweep();
}

bool BZ2File::open(const String& filename, const String& mode) {
  assert(m_bzFile == nullptr);

  return m_innerFile->open(filename, mode) &&
    (m_bzFile = BZ2_bzdopen(dup(m_innerFile->fd()), mode.data()));
}

bool BZ2File::close() {
  return closeImpl();
}

int64_t BZ2File::errnu() {
  assert(m_bzFile);
  int errnum = 0;
  BZ2_bzerror(m_bzFile, &errnum);
  return errnum;
}

String BZ2File::errstr() {
  assert(m_bzFile);
  int errnum;
  return BZ2_bzerror(m_bzFile, &errnum);
}

const StaticString
  s_errno("errno"),
  s_errstr("errstr");

Variant BZ2File::error() {
  assert(m_bzFile);
  int errnum;
  const char * errstr;
  errstr = BZ2_bzerror(m_bzFile, &errnum);
  return make_map_array(s_errno, errnum, s_errstr, String(errstr));
}

bool BZ2File::flush() {
  assert(m_bzFile);
  return BZ2_bzflush(m_bzFile);
}

int64_t BZ2File::readImpl(char * buf, int64_t length) {
  assert(m_bzFile);
  int len = BZ2_bzread(m_bzFile, buf, length);
  if (len < length)
    m_eof = true;
  return len;
}

int64_t BZ2File::writeImpl(const char * buf, int64_t length) {
  assert(m_bzFile);
  return BZ2_bzwrite(m_bzFile, (char *)buf, length);
}

bool BZ2File::closeImpl() {
  assert(m_bzFile);
  bool ret = true;
  BZ2_bzclose(m_bzFile);
  m_bzFile = nullptr;
  m_innerFile->close();
  File::closeImpl();
  m_eof = false;
  return ret;
}

bool BZ2File::eof() {
  assert(m_bzFile);
  return m_eof;
}

///////////////////////////////////////////////////////////////////////////////
}
