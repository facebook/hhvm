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
#include "hphp/runtime/ext/bz2/bz2-file.h"
#include "hphp/runtime/base/array-init.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

BZ2File::BZ2File(): m_bzFile(nullptr), m_innerFile(req::make<PlainFile>()) {
  m_innerFile->unregister();
  setIsLocal(m_innerFile->isLocal());
}

BZ2File::BZ2File(req::ptr<PlainFile>&& innerFile)
: m_bzFile(nullptr), m_innerFile(std::move(innerFile)) {
  setIsLocal(m_innerFile->isLocal());
}

BZ2File::~BZ2File() {
  closeImpl();
}

void BZ2File::sweep() {
  closeImpl();
  File::sweep();
}

bool BZ2File::open(const String& filename, const String& mode) {
  assertx(m_bzFile == nullptr);

  return m_innerFile->open(filename, mode) &&
    (m_bzFile = BZ2_bzdopen(dup(m_innerFile->fd()), mode.data()));
}

bool BZ2File::close() {
  return closeImpl();
}

int64_t BZ2File::errnu() {
  assertx(m_bzFile);
  int errnum = 0;
  BZ2_bzerror(m_bzFile, &errnum);
  return errnum;
}

String BZ2File::errstr() {
  assertx(m_bzFile);
  int errnum;
  return BZ2_bzerror(m_bzFile, &errnum);
}

const StaticString
  s_errno("errno"),
  s_errstr("errstr");

Array BZ2File::error() {
  assertx(m_bzFile);
  int errnum;
  const char * errstr;
  errstr = BZ2_bzerror(m_bzFile, &errnum);
  return make_dict_array(s_errno, errnum, s_errstr, String(errstr));
}

bool BZ2File::flush() {
  assertx(m_bzFile);
  return BZ2_bzflush(m_bzFile);
}

int64_t BZ2File::readImpl(char * buf, int64_t length) {
  if (length == 0) {
    return 0;
  }
  assertx(m_bzFile);
  int len = BZ2_bzread(m_bzFile, buf, length);
  /* Sometimes libbz2 will return fewer bytes than requested, and set bzerror
   * to BZ_STREAM_END, but it's not actually EOF, and you can keep reading from
   * the file - so, only set EOF after a failed read. This matches PHP5.
   */
  if (len <= 0) {
    setEof(true);
    if (len < 0) {
      return 0;
    }
  }
  return len;
}

int64_t BZ2File::writeImpl(const char * buf, int64_t length) {
  assertx(m_bzFile);
  return BZ2_bzwrite(m_bzFile, (char *)buf, length);
}

bool BZ2File::closeImpl() {
  if (!isClosed()) {
    if (m_bzFile) {
      BZ2_bzclose(m_bzFile);
      m_bzFile = nullptr;
    }
    setIsClosed(true);
    if (m_innerFile) {
      m_innerFile->close();
    }
  }
  File::closeImpl();
  return true;
}

bool BZ2File::eof() {
  assertx(m_bzFile);
  return getEof();
}

///////////////////////////////////////////////////////////////////////////////
}
