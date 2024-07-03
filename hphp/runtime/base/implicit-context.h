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
#include "hphp/runtime/base/req-vector.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP {

struct ImplicitContext {

  ////////////////////////////////////////////////////////////////////////////
  // Members
  ////////////////////////////////////////////////////////////////////////////

  /*
   * Holds an int memo key so that all IC objects with the same
   * instance keys will map to the same memo key. The map from string
   * instance keys to this memo key is maintained in ext_implicit_context
  */
  int64_t m_memoKey;

  // HashMap of TypedValues and their instance keys
  req::fast_map<const StringData*, std::pair<TypedValue, TypedValue>,
                string_data_hash, string_data_same> m_map;

  ////////////////////////////////////////////////////////////////////////////
  // Statics
  ////////////////////////////////////////////////////////////////////////////

  static rds::Link<ObjectData*, rds::Mode::Normal> activeCtx;
  static rds::Link<ObjectData*, rds::Mode::Normal> emptyCtx;

  static Variant getBlameVectors();

  static void setActive(Object&&);

  static constexpr ptrdiff_t memoKeyOffset() {
    return offsetof(ImplicitContext, m_memoKey);
  }

  ////////////////////////////////////////////////////////////////////////////
  // RAII wrappers
  ////////////////////////////////////////////////////////////////////////////

  /*
  * RAII wrapper for saving implicit context
  */
  struct Saver {
    Saver();
    ~Saver();

  private:
    ObjectData* m_context;
  };
};

} // namespace HPHP
