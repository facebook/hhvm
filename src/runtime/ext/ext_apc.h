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

#ifndef __EXT_APC_H__
#define __EXT_APC_H__

#include <runtime/base/base_includes.h>
#include <runtime/base/shared/shared_store.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool f_apc_add(CStrRef key, CVarRef var, int64 ttl = 0, int64 cache_id = 0);
bool f_apc_store(CStrRef key, CVarRef var, int64 ttl = 0, int64 cache_id = 0);
Variant f_apc_fetch(CVarRef key, Variant success = null, int64 cache_id = 0);
Variant f_apc_delete(CVarRef key, int64 cache_id = 0);
bool f_apc_clear_cache(int64 cache_id = 0);
Variant f_apc_inc(CStrRef key, int64 step = 1, Variant success = null, int64 cache_id = 0);
Variant f_apc_dec(CStrRef key, int64 step = 1, Variant success = null, int64 cache_id = 0);
bool f_apc_cas(CStrRef key, int64 old_cas, int64 new_cas, int64 cache_id = 0);

///////////////////////////////////////////////////////////////////////////////

Variant f_apc_cache_info(int64 cache_id = 0, bool limited = false);
inline Array f_apc_sma_info(bool limited = false) {
  return Array::Create();
}
inline bool f_apc_define_constants(CStrRef key, CStrRef constants,
                                   bool case_sensitive = true,
                                   int64 cache_id = 0) {
  throw NotSupportedException(__func__, "dynamic coding");
}
inline bool f_apc_load_constants(CStrRef key, bool case_sensitive = true,
                                 int64 cache_id = 0) {
  throw NotSupportedException(__func__, "dynamic coding");
}
inline bool f_apc_compile_file(CStrRef filename, bool atomic = true,
                               int64 cache_id = 0) {
  throw NotSupportedException(__func__, "dynamic coding");
}
inline Array f_apc_filehits() {
  throw NotSupportedException(__func__, "feature not supported");
}
inline Variant f_apc_delete_file(CVarRef keys, int64 cache_id = 0) {
  throw NotSupportedException(__func__, "feature not supported");
}
inline Variant f_apc_bin_dump(int64 cache_id = 0, CVarRef filter = null_variant) {
  throw NotSupportedException(__func__, "feature not supported");
}
inline bool f_apc_bin_load(CStrRef data, int64 flags = 0, int64 cache_id = 0) {
  throw NotSupportedException(__func__, "feature not supported");
}
inline Variant f_apc_bin_dumpfile(int64 cache_id, CVarRef filter,
                                  CStrRef filename, int64 flags = 0,
                                  CObjRef context = null) {
  throw NotSupportedException(__func__, "feature not supported");
}
inline bool f_apc_bin_loadfile(CStrRef filename, CObjRef context = null,
                               int64 flags = 0, int64 cache_id = 0) {
  throw NotSupportedException(__func__, "feature not supported");
}

///////////////////////////////////////////////////////////////////////////////
// loading APC from archive files

void apc_load(int thread);

// needed by generated apc archive .cpp files
void apc_load_impl(const char **int_keys, int64 *int_values,
                   const char **char_keys, char *char_values,
                   const char **strings, const char **objects,
                   const char **thrifts, const char **others);

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_APC_H__
