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

#include "hphp/runtime/base/user-stream-wrapper.h"
#include "hphp/runtime/base/user-file.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

UserStreamWrapper::UserStreamWrapper(const String& name,
                                     const String& clsname)
    : m_name(name) {
  m_cls = Unit::loadClass(clsname.get());
  if (!m_cls) {
    throw InvalidArgumentException(0, "Undefined class '%s'", clsname.data());
  }
  // There is a third param in Zend to stream_wrapper_register() which could
  // affect that when we add that param
  m_isLocal = true;
}

File* UserStreamWrapper::open(const String& filename, const String& mode,
                              int options, CVarRef context) {
  std::unique_ptr<File> file(NEWOBJ(UserFile)(m_cls, options, context));
  file->open(filename, mode);
  return file.release();
}

///////////////////////////////////////////////////////////////////////////////
}
