/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/base/output_file.h"
#include "hphp/runtime/base/complex_types.h"
#include "hphp/runtime/base/execution_context.h"
#include "hphp/runtime/base/runtime_error.h"

namespace HPHP {

IMPLEMENT_OBJECT_ALLOCATION(OutputFile)
///////////////////////////////////////////////////////////////////////////////

StaticString OutputFile::s_class_name("OutputFile");

///////////////////////////////////////////////////////////////////////////////
// constructor and destructor

OutputFile::OutputFile(CStrRef filename) {
  if (filename != "php://output") {
    throw FatalErrorException("not a php://output file ");
  }
  m_isLocal = true;
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
  s_file_data->m_pcloseRet = 0;
  if (!m_closed) {
    m_closed = true;
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// virtual functions

int64_t OutputFile::readImpl(char *buffer, int64_t length) {
  raise_warning("cannot read from a php://output stream");
  return -1;
}

int OutputFile::getc() {
  raise_warning("cannot read from a php://output stream");
  return -1;
}

int64_t OutputFile::writeImpl(const char *buffer, int64_t length) {
  assert(length > 0);
  if (m_closed) return 0;
  g_context->write(buffer, length);
  return length;
}

bool OutputFile::seek(int64_t offset, int whence /* = SEEK_SET */) {
  raise_warning("cannot seek a php://output stream");
  return false;
}

int64_t OutputFile::tell() {
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
    g_context->flush();
    return true;
  }
  return false;
}

bool OutputFile::truncate(int64_t size) {
  raise_warning("cannot truncate a php://output stream");
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}
