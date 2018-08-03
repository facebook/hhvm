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

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/typed-value.h"

#include "hphp/util/compact-tagged-ptrs.h"
#include "hphp/util/compilation-flags.h"

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
  static T dummy() { return T { &immutable_uninit_base }; }
  bool is_dummy() const { return static_cast<const T&>(*this) == dummy(); }
};

template<typename T>
T* get_ptr(T* ptr) {
  return ptr;
}

template<typename T, typename Tag>
T* get_ptr(CompactTaggedPtr<T, Tag> ptr) {
  return ptr.ptr();
}
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
 *
 * If tag_t is non-void, CompactTaggedPtr will be used internally to store a
 * tag. This has no space overhead, but has a slight penalty at runtime.
 */
template<bool is_const, typename tag_t = void>
struct tv_val : std::conditional<is_const,
                                 tv_val_detail::with_dummy<tv_val<true>>,
                                 tv_val_detail::empty>::type {
private:
  template<typename T> using maybe_const_t =
    typename std::conditional<is_const, const T, T>::type;
  template<typename T, typename R = T> using with_tag_t =
    typename std::enable_if<!std::is_same<T, void>::value, R>::type;

public:
  using value_t = maybe_const_t<Value>;
  using type_t = maybe_const_t<DataType>;
  using tv_t = maybe_const_t<TypedValue>;

  /*
   * These values expose details about the internal representation of a tv_val,
   * and should only be inspected while generating code that works with
   * tv_vals.
   *
   * If you do change these, you must also update the return registers used in
   * runtime/base/hash-table-*.S.
   */
  static constexpr int type_idx = wide_tv_val ? 0 : -1;
  static constexpr int val_idx = wide_tv_val ? 1 : 0;

  tv_val();
  /* implicit */ tv_val(tv_t* lval);
  tv_val(type_t* type, value_t* val);

  /*
   * Construct from a tv_val without a tag and a tag.
   */
  template<typename Tag = tag_t>
  tv_val(tv_val<is_const> lval, with_tag_t<Tag> t);

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

  template<typename Tag = tag_t>
  with_tag_t<Tag> tag() const;

  template<typename Tag = tag_t>
  with_tag_t<Tag, tv_val<is_const>> drop_tag() const;

private:
  template<bool, typename> friend struct tv_val;

  template<typename T>
  using maybe_tagged_t = std::conditional_t<
    std::is_same<tag_t, void>::value, T*, CompactTaggedPtr<T, tag_t>
  >;

  /*
   * Default storage type: a single TypedValue*.
   */
  struct storage {
    storage(type_t* type, value_t* val)
      : m_tv{reinterpret_cast<tv_t*>(val)}
    {
      assertx(val == nullptr || &m_tv->m_type == type);
    }

    template<typename Tag = tag_t>
    storage(type_t* type, value_t* val, with_tag_t<Tag> tag)
      : m_tv{tag, reinterpret_cast<tv_t*>(val)}
    {
      assertx(val == nullptr || &m_tv->m_type == type);
    }

    bool operator==(const storage& o) const {
      return m_tv == o.m_tv;
    }

    type_t* type() const { return &m_tv->m_type; }
    value_t* val() const { return &m_tv->m_data; }

    template<typename Tag = tag_t>
    with_tag_t<Tag> tag() const { return m_tv.tag(); }

   private:
    maybe_tagged_t<tv_t> m_tv;
  };

  /*
   * Wide storage type: separate pointers for the type and the value. m_type is
   * only meangingful is m_val != nullptr.
   */
  struct wide_storage {
    wide_storage(type_t* type, value_t* val)
      : m_type{type}
      , m_val{val}
    {
      assertx((type && val) || val == nullptr);
    }

    template<typename Tag = tag_t>
    wide_storage(type_t* type, value_t* val, with_tag_t<Tag> tag)
      : m_type{tag, type}
      , m_val{val}
    {
      assertx((type && val) || val == nullptr);
    }

    bool operator==(const wide_storage& o) const {
      return m_val == o.m_val && (m_type == o.m_type || m_val == nullptr);
    }

    type_t* type() const { return tv_val_detail::get_ptr(m_type); }
    value_t* val() const { return m_val; }

    template<typename Tag = tag_t>
    with_tag_t<Tag> tag() const { return m_type.tag(); }

   private:
    maybe_tagged_t<type_t> m_type;
    value_t* m_val;
  };

  using storage_t = std::conditional_t<wide_tv_val, wide_storage, storage>;
  storage_t m_s;
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
