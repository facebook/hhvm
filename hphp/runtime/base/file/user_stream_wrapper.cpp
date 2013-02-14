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

#include <runtime/base/file/user_stream_wrapper.h>
#include <runtime/base/file/user_file.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

UserStreamWrapper::UserStreamWrapper(CStrRef name, CStrRef clsname) :
  m_name(name) {
  if (!hhvm) {
    throw NotSupportedException("user-streams", "Not supported under HPHPc");
  }
  m_cls = VM::Unit::loadClass(clsname.get());
  if (!m_cls) {
    throw InvalidArgumentException(0, "Undefined class '%s'", clsname.data());
  }
}

File* UserStreamWrapper::open(CStrRef filename, CStrRef mode,
                              int options, CVarRef context) {
  std::unique_ptr<File> file(NEWOBJ(UserFile)(m_cls, options, context));
  file->open(filename, mode);
  return file.release();
}

///////////////////////////////////////////////////////////////////////////////
}
