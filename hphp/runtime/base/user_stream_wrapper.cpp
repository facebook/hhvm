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

#include "hphp/runtime/base/user_stream_wrapper.h"
#include "hphp/runtime/base/user_file.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

UserStreamWrapper::UserStreamWrapper(CStrRef name, CStrRef clsname) :
  m_name(name) {
  m_cls = Unit::loadClass(clsname.get());
  if (!m_cls) {
    throw InvalidArgumentException(0, "Undefined class '%s'", clsname.data());
  }
  // There is a third param in Zend to stream_wrapper_register() which could
  // affect that when we add that param
  m_isLocal = true;
}

File* UserStreamWrapper::open(CStrRef filename, CStrRef mode,
                              int options, CVarRef context) {
  std::unique_ptr<File> file(NEWOBJ(UserFile)(m_cls, options, context));
  file->open(filename, mode);
  return file.release();
}

///////////////////////////////////////////////////////////////////////////////
}
