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

#include "hphp/runtime/base/output-file.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/runtime-error.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// constructor and destructor

const StaticString s_php_output("php://output");
const StaticString s_php("PHP");
const StaticString s_output("Output");

OutputFile::OutputFile(const String& filename): File(true, s_php, s_output) {
  if (filename != s_php_output) {
    throw FatalErrorException("not a php://output file ");
  }
  m_isLocal = true;
}

OutputFile::~OutputFile() {
  OutputFile::closeImpl();
}

void OutputFile::sweep() {
  closeImpl();
  File::sweep();
}

bool OutputFile::open(const String& filename, const String& mode) {
  throw FatalErrorException("cannot open a php://output file ");
}

bool OutputFile::close() {
  invokeFiltersOnClose();
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
