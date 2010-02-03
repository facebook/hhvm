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

#include <cpp/base/string_offset.h>
#include <cpp/base/type_string.h>
#include <cpp/base/type_variant.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

StringOffset::operator String() const {
  return m_data->getChar(m_offset);
}

StringOffset &StringOffset::operator=(CVarRef v) {
  m_data->setChar(m_offset, v.toString());
  return *this;
}

///////////////////////////////////////////////////////////////////////////////
}
