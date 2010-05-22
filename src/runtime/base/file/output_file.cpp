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

#include <runtime/base/file/output_file.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/execution_context.h>
#include <runtime/base/runtime_error.h>

namespace HPHP {

IMPLEMENT_OBJECT_ALLOCATION(OutputFile);
///////////////////////////////////////////////////////////////////////////////
// constructor and destructor

OutputFile::OutputFile(CStrRef filename) {
  if (filename != "php://output") {
    throw FatalErrorException("not a php://output file ");
  }
}

OutputFile::~OutputFile() {
  closeImpl();
}

bool OutputFile::open(CStrRef filename, CStrRef mode) {
  throw FatalErrorException("cannot open a php://output file ");
}

bool OutputFile::close() {
  return closeImpl();
}

bool OutputFile::closeImpl() {
  if (!m_closed) {
    m_closed = true;
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// virtual functions

int OutputFile::readImpl(char *buffer, int length) {
  raise_warning("cannot read from a php://output stream");
  return -1;
}

int OutputFile::getc() {
  raise_warning("cannot read from a php://output stream");
  return -1;
}

int OutputFile::writeImpl(const char *buffer, int length) {
  ASSERT(length > 0);
  if (m_closed) return 0;
  g_context->out().write(buffer, length);
  return length;
}

bool OutputFile::seek(int offset, int whence /* = SEEK_SET */) {
  raise_warning("cannot seek a php://output stream");
  return false;
}

int OutputFile::tell() {
  raise_warning("cannot tell a php://output stream");
  return -1;
}

bool OutputFile::eof() {
  return false;
}

bool OutputFile::rewind() {
  raise_warning("cannot rewind a php://output stream");
  return false;
}

bool OutputFile::flush() {
  if (!m_closed) {
    (g_context->out()).flush();
    return true;
  }
  return false;
}

bool OutputFile::truncate(int size) {
  raise_warning("cannot truncate a php://output stream");
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}
