/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/server/server-note.h"
#include "hphp/runtime/base/request-local.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static THREAD_LOCAL_NO_CHECK(ServerNote, s_note);

ServerNote* get_server_note() {
  return s_note.getCheck();
}

void ServerNote::Add(const String& name, const String& value) {
  Array &arr = s_note->m_notes;
  arr.set(name, value);
}

String ServerNote::Get(const String& name) {
  Array &arr = s_note->m_notes;
  return arr.exists(name)
    ? tvCastToString(arr.rvalAt(name).tv())
    : String{};
}

void ServerNote::Delete(const String& name) {
  s_note->m_notes.remove(name);
}

void ServerNote::Reset() {
   s_note->m_notes.reset();
}

///////////////////////////////////////////////////////////////////////////////
}
