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

  // HashMap of TypedValues and their instance keys, for memo agnostic
  // values, the second.second TypedValue will be KindOfUninit
  req::fast_map<const Class*, std::pair<TypedValue, TypedValue>> m_map;

  /*
   * How Implicit Contexts behave with respect to memoization
   * divides them into two varieties: memo-agnostic and memo-sensitive.
   * Memo-agnostic ICs do not affect memoization
   * Memo-sensitive ICs contribute to memoization sharding
   * We are implementing it in a way that at any given time, the current
   * activeCtx will contain the information in the m_map that is
   * consistent with its memoization type. To accomoplish this, we
   * keep a second pointer called m_memoAgnosticIC in the struct that
   * contains only memo agnostic keys
   * Additionally, whenever we need to be in the memo agnostic type IC,
   * we can do so simply by swapping the activeCtx with the current IC's
   * memo agnostic counterpart. Note that there is no variable to hold
   * the current 'State' since the 'State' is determined by what is stored
   * in m_map
  */
  ObjectData* m_memoAgnosticIC;

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

// We only transition from memo-sensitive to memo-agnostic
// in try catch blocks that restore the memo-sensitive ctx
static constexpr ptrdiff_t memoAgnosticOffset() {
  return offsetof(ImplicitContext, m_memoAgnosticIC);
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
