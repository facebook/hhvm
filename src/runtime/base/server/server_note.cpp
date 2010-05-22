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
#include <runtime/base/server/server_note.h>
#include <runtime/base/util/request_local.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static IMPLEMENT_THREAD_LOCAL(ServerNote, s_note);

void ServerNote::Add(CStrRef name, CStrRef value) {
  Array &arr = s_note->m_notes;
  arr.set(name, value);
}

String ServerNote::Get(CStrRef name) {
  Array &arr = s_note->m_notes;
  String ret;
  if (arr.exists(name)) {
    ret = arr.rvalAt(name);
  }
  return ret;
}

void ServerNote::Reset() {
   s_note->m_notes.reset();
}

///////////////////////////////////////////////////////////////////////////////
}
