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

#include <runtime/base/debug/backtrace.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/source_info.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Array stackTraceToBackTrace(const StackTrace& st) {
  std::vector<void*> bt_pointers;
  st.get(bt_pointers);
  Array ret;
  if (RuntimeOption::FullBacktrace) {
    for (unsigned int i = 0; i < bt_pointers.size(); i++) {
      StackTrace::FramePtr f = StackTrace::Translate(bt_pointers[i]);
      if (RuntimeOption::TranslateSource) {
        SourceInfo::TheSourceInfo.translate(f);
      }
      Array frame;
      frame.set("file",     String(f->filename));
      frame.set("line",     f->lineno);
      frame.set("function", String(f->funcname));
      frame.set("args",     "");
      frame.set("bt",       (int64)bt_pointers[i]);
      ret.append(frame);
    }
  } else {
    for (unsigned int i = 0; i < bt_pointers.size(); i++) {
      Array frame;
      frame.set("file",     "");
      frame.set("line",     0LL);
      frame.set("function", "");
      frame.set("args",     "");
      frame.set("bt",       (int64)bt_pointers[i]);
      ret.append(frame);
    }
    ret.set("bts", String(st.hexEncode()));
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
