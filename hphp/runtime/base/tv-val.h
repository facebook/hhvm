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

#ifndef incl_HPHP_MEMB_VAL_H_
#define incl_HPHP_MEMB_VAL_H_

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/typed-value.h"

#include <cstddef>
#include <type_traits>

namespace HPHP {

namespace tv_val_detail {
/*
 * These structs are used to add dummy() and is_dummy() functions to tv_rval
 * only.
 */
struct empty {};

template<typename T>
struct with_dummy {
  static T dummy() { return T { &immutable_uninit_base }; }
  bool is_dummy() const { return static_cast<const T&>(*this) == dummy(); }
};
}

/*
 * NOTE: You probably do not want to use tv_val directly. You should instead
 * use tv_lval or tv_rval, for mutable or constant pointees, respectively.
 *
 * tv_val is a thin wrapper around the concept of a pointer to a
 * TypedValue. For now, it contains a normal TypedValue*, but in the future we
 * will explore different memory layout options that will likely require
 * passing around separate pointers to the value and type of a TypedValue. The
 * goal of tv_val is to replace TypedValue* in as much code as possible, so
 * these alternate layout options can be explored with minimal disruption.
 *
 * Like all pointers, tv_val is nullable/optional. The presence of a value can
 * be detected via is_set(), explicit cast to a bool, or comparison with
 * nullptr.
 */
template<bool is_const>
struct tv_val : std::conditional<is_const,
                                 tv_val_detail::with_dummy<tv_val<true>>,
                                 tv_val_detail::empty>::type {
private:
  template<typename T> using maybe_const_t =
    typename std::conditional<is_const, const T, T>::type;

public:
  using value_t = maybe_const_t<Value>;
  using type_t = maybe_const_t<DataType>;
  using tv_t = maybe_const_t<TypedValue>;

  tv_val();
  /* implicit */ tv_val(tv_t* lval);

  bool operator==(tv_val other) const;

  /*
   * Whether this tv_val is set.
   */
  bool is_set() const;
  explicit operator bool() const;
  bool operator==(std::nullptr_t) const;
  bool operator!=(std::nullptr_t) const;

  /*
   * Implicit cast to tv_rval.
   */
  /* implicit */ operator tv_val<true>() const;

  /*
   * Explicit cast to tv_lval.
   *
   * This is the moral equivalent of:
   *   const_cast<TypedValue*>(const TypedValue*)
   */
  tv_val<false> as_lval() const;

  /*
   * References to the value and type.
   *
   * @requires: is_set()
   */
  value_t& val() const;
  type_t& type() const;

  /*
   * Get a pointer to the referenced TypedValue.
   *
   * Behavior is undefined if this object was not constructed from a single
   * TypedValue*. Currently, this never happens.
   *
   * TODO(#9077255): Make it happen for monomorphic arrays.
   */
  tv_t* tv_ptr() const;

  /*
   * Get a copy of the referenced value and type as a TypedValue.
   *
   * @requires: is_set()
   */
  TypedValue tv() const;
  TypedValue operator*() const;


  /*
   * Return `this' if the referenced value is already unboxed, else a tv_val to
   * the inner value.
   */
  tv_val unboxed() const;

  /*
   * The canonical non-null "missing" rval. Only valid for tv_rval (is_const ==
   * true). These are actually defined in tv_val_detail::with_dummy; see above.
   *
   * Some users of tv_rval prefer to use a dummy rval-to-Uninit to represent a
   * missing element, instead of a nullptr rval, so that tv() is always valid.
   * These functions provide and test for such a value.
   *
   * static tv_val dummy();
   * bool is_dummy() const;
   */

private:
  tv_t* m_tv;
};

/*
 * TV-lval API for tv_val.
 */
template<bool is_const>
auto& type(const tv_val<is_const>& val) { return val.type(); }
template<bool is_const>
auto& val(const tv_val<is_const>& val) { return val.val(); }
template<bool is_const>
TypedValue as_tv(const tv_val<is_const>& val) { return val.tv(); }

///////////////////////////////////////////////////////////////////////////////

using tv_lval = tv_val<false>;
using tv_rval = tv_val<true>;

///////////////////////////////////////////////////////////////////////////////

}

#include "hphp/runtime/base/tv-val-inl.h"

#endif
