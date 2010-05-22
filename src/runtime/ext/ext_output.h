/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#ifndef __EXT_OUTPUT_H__
#define __EXT_OUTPUT_H__

#include <runtime/base/base_includes.h>
#include <runtime/base/server/server_stats.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline bool f_ob_start(CVarRef output_callback = null, int chunk_size = 0,
                       bool erase = true) {
  // ignoring chunk_size and erase
  g_context->obStart(output_callback);
  return true;
}
inline void f_ob_clean() {
  g_context->obClean();
}
inline void f_ob_flush() {
  g_context->obFlush();
}
inline bool f_ob_end_clean() {
  g_context->obClean();
  g_context->obEnd();
  return true;
}
inline bool f_ob_end_flush() {
  bool ret = g_context->obFlush();
  g_context->obEnd();
  return ret;
}
inline void f_flush() {
  g_context->flush();
}
inline String f_ob_get_contents() {
  return g_context->obGetContents();
}
inline String f_ob_get_clean() {
  String output = f_ob_get_contents();
  f_ob_end_clean();
  return output;
}
inline String f_ob_get_flush() {
  String output = g_context->obGetContents();
  g_context->obFlush();
  return output;
}
inline int f_ob_get_length() {
  return g_context->obGetContentLength();
}
inline int f_ob_get_level() {
  return g_context->obGetLevel();
}
inline Array f_ob_get_status(bool full_status = false) {
  throw NotSupportedException(__func__, "not useful");
}
inline String f_ob_gzhandler(CStrRef buffer, int mode) {
  throw NotSupportedException(__func__, "something that's in transport layer");
}
inline void f_ob_implicit_flush(bool flag = true) {
  g_context->obSetImplicitFlush(flag);
}
inline Array f_ob_list_handlers() {
  return g_context->obGetHandlers();
}
inline bool f_output_add_rewrite_var(CStrRef name, CStrRef value) {
  throw NotSupportedException(__func__, "bad coding style");
}
inline bool f_output_reset_rewrite_vars() {
  throw NotSupportedException(__func__, "bad coding style");
}

bool f_hphp_log(CStrRef filename, CStrRef message);

inline void f_hphp_stats(CStrRef name, int64 value) {
  ServerStats::Log(name.data(), value);
}
inline int64 f_hphp_get_stats(CStrRef name) {
  return ServerStats::Get(name.data());
}
inline void f_hphp_output_global_state(CStrRef filename = null_string) {
  FILE *fp = NULL;
  if (!filename.isNull()) {
    fp = fopen(filename.c_str(), "w");
    if (!fp) {
      throw Exception("failed to open %s for writing", filename.c_str());
    }
  }
  output_global_state(fp);
  if (fp) fclose(fp);
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_OUTPUT_H__
