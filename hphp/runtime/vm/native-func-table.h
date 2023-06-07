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
#pragma once

#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/base/string-functors.h"
#include "hphp/util/hash-map.h"

namespace HPHP::Native {

struct FuncTable {
  void insert(const StringData* name, const NativeFunctionInfo&);
  NativeFunctionInfo get(const StringData* name) const;
  void dump() const;
  bool empty() const { return m_infos.empty(); }
 private:
  // This is intentionally case sensitive despite classes and
  // functions being case insensitive. Binding native impls to HNI
  // decls is an internal HHVM operation and must be case-correct.
  hphp_hash_map<const StringData*, NativeFunctionInfo,
                string_data_hash, string_data_same> m_infos;
};

}
