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
#ifndef incl_HPHP_RUNTIME_VM_NATIVE_FUNC_TABLE_H
#define incl_HPHP_RUNTIME_VM_NATIVE_FUNC_TABLE_H

#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/base/string-functors.h"
#include "hphp/util/hash-map.h"

namespace HPHP { namespace Native {

struct FuncTable {
  void insert(const StringData* name, const NativeFunctionInfo&);
  NativeFunctionInfo get(const StringData* name) const;
  void dump() const;
 private:
  hphp_hash_map<const StringData*, NativeFunctionInfo,
                string_data_hash, string_data_isame> m_infos;
};

}}
#endif
