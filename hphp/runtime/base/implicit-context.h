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

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/req-hash-map.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

struct ImplicitContext {

static rds::Link<ObjectData*, rds::Mode::Normal> activeCtx;

// Combination of the instance keys
StringData* m_memokey;

// HashMap of TypedValues and their instance keys
req::fast_map<const StringData*, std::pair<TypedValue, TypedValue>,
              string_data_hash, string_data_same> m_map;

static Object setByValue(Object&&);

/*
 * RAII wrapper for saving implicit context
 */
struct Saver {
  Saver() {
    if (RO::EvalEnableImplicitContext) {
      m_context = *ImplicitContext::activeCtx;
      *ImplicitContext::activeCtx = nullptr;
    }
  }
  ~Saver() {
    if (RO::EvalEnableImplicitContext) *ImplicitContext::activeCtx = m_context;
  }

private:
  ObjectData* m_context;
};

};

} // namespace HPHP

