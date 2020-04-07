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
  INLINE_FLATTEN static T dummy() { return T { &immutable_uninit_base }; }
  INLINE_FLATTEN bool is_dummy() const {
    return static_cast<const T&>(*this) == dummy();
  }
};

template<typename T>
INLINE_FLATTEN T* get_ptr(T* ptr) {
  return ptr;
}

template<typename T, typename Tag>
INLINE_FLATTEN T* get_ptr(CompactTaggedPtr<T, Tag> ptr) {
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

  INLINE_FLATTEN tv_val();
  /* implicit */ INLINE_FLATTEN tv_val(std::nullptr_t);
  /* implicit */ INLINE_FLATTEN tv_val(tv_t* lval);
  INLINE_FLATTEN tv_val(type_t* type, value_t* val);

  /*
   * Construct from a tv_val without a tag and a tag.
   */
  template<typename Tag = tag_t>
  INLINE_FLATTEN tv_val(tv_val<is_const> lval, with_tag_t<Tag> t);

  INLINE_FLATTEN bool operator==(tv_val other) const;
  INLINE_FLATTEN bool operator!=(tv_val other) const;

  /*
   * Whether this tv_val is set.
   */
  INLINE_FLATTEN bool is_set() const;
  INLINE_FLATTEN explicit operator bool() const;
  INLINE_FLATTEN bool operator==(std::nullptr_t) const;
  INLINE_FLATTEN bool operator!=(std::nullptr_t) const;

  /*
   * Implicit cast to tv_rval.
   */
  /* implicit */ INLINE_FLATTEN operator tv_val<true>() const;

  /*
   * Explicit cast to tv_lval.
   *
   * This is the moral equivalent of:
   *   const_cast<TypedValue*>(const TypedValue*)
   */
  INLINE_FLATTEN tv_val<false> as_lval() const;

  /*
   * References to the value and type.
   *
   * @requires: is_set()
   */
  INLINE_FLATTEN value_t& val() const;
  INLINE_FLATTEN type_t& type() const;

  /*
   * Get a copy of the referenced value and type as a TypedValue.
   *
   * @requires: is_set()
   */
  INLINE_FLATTEN TypedValue tv() const;
  INLINE_FLATTEN TypedValue operator*() const;

  template<typename Tag = tag_t>
  INLINE_FLATTEN with_tag_t<Tag> tag() const;

  template<typename Tag = tag_t>
  INLINE_FLATTEN with_tag_t<Tag, tv_val<is_const>> drop_tag() const;

  TYPE_SCAN_CUSTOM() {
    if (isRefcountedType(type())) scanner.scan(val().pcnt);
  }

  static constexpr size_t typeOff() {
    return offsetof(tv_val, m_s) + storage_t::typeOff();
  }
  static constexpr size_t dataOff() {
    return offsetof(tv_val, m_s) + storage_t::dataOff();
  }
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
    INLINE_FLATTEN storage()
      : m_tv{} {}

    INLINE_FLATTEN storage(type_t* type, value_t* val)
      : m_tv{reinterpret_cast<tv_t*>(val)}
    {
      assertx(val == nullptr || &m_tv->m_type == type);
    }

    template<typename Tag = tag_t>
    INLINE_FLATTEN storage(type_t* type, value_t* val, with_tag_t<Tag> tag)
      : m_tv{tag, reinterpret_cast<tv_t*>(val)}
    {
      assertx(val == nullptr || &m_tv->m_type == type);
    }

    INLINE_FLATTEN bool operator==(const storage& o) const {
      return m_tv == o.m_tv;
    }

    INLINE_FLATTEN bool operator!=(const storage& o) const {
      return m_tv != o.m_tv;
    }

    INLINE_FLATTEN type_t* type() const { return &m_tv->m_type; }
    INLINE_FLATTEN value_t* val() const { return &m_tv->m_data; }
    INLINE_FLATTEN bool is_set() const { return static_cast<bool>(m_tv); }

    template<typename Tag = tag_t>
    INLINE_FLATTEN with_tag_t<Tag> tag() const { return m_tv.tag(); }

    static constexpr size_t typeOff() {
      return offsetof(storage, m_tv);
    }
    static constexpr size_t dataOff() {
      return offsetof(storage, m_tv);
    }

   private:
    maybe_tagged_t<tv_t> m_tv;
  };

  /*
   * Wide storage type: separate pointers for the type and the value. m_type is
   * only meangingful is m_val != nullptr.
   */
  struct wide_storage {
    INLINE_FLATTEN wide_storage()
        : m_type{}, m_val{} {}

    INLINE_FLATTEN wide_storage(type_t* type, value_t* val)
      : m_type{type}
      , m_val{val}
    {
      assertx((type && val) || val == nullptr);
    }

    template<typename Tag = tag_t>
    INLINE_FLATTEN wide_storage(type_t* type, value_t* val, with_tag_t<Tag> tag)
      : m_type{tag, type}
      , m_val{val}
    {
      assertx((type && val) || val == nullptr);
    }

    INLINE_FLATTEN bool operator==(const wide_storage& o) const {
      return m_val == o.m_val && (m_type == o.m_type || m_val == nullptr);
    }

    INLINE_FLATTEN bool operator!=(const wide_storage& o) const {
      return !operator==(o);
    }

    INLINE_FLATTEN type_t* type() const {
      return tv_val_detail::get_ptr(m_type);
    }
    INLINE_FLATTEN value_t* val() const { return m_val; }
    INLINE_FLATTEN bool is_set() const { return m_val; }

    template<typename Tag = tag_t>
    INLINE_FLATTEN with_tag_t<Tag> tag() const { return m_type.tag(); }

    static constexpr size_t typeOff() {
      return offsetof(wide_storage, m_type);
    }
    static constexpr size_t dataOff() {
      return offsetof(wide_storage, m_val);
    }

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
INLINE_FLATTEN auto& type(const tv_val<is_const>& val) { return val.type(); }
template<bool is_const>
INLINE_FLATTEN auto& val(const tv_val<is_const>& val) { return val.val(); }
template<bool is_const>
INLINE_FLATTEN TypedValue as_tv(const tv_val<is_const>& val) {
  return val.tv();
}

///////////////////////////////////////////////////////////////////////////////

using tv_lval = tv_val<false>;
using tv_rval = tv_val<true>;

///////////////////////////////////////////////////////////////////////////////

namespace detail {

/* representation of a tv_val_offset when wide mode is on */
struct tv_val_offset_wide {
  tv_val_offset_wide(ptrdiff_t tv_offset)
    : type_offset(tv_offset + offsetof(TypedValue, m_type))
    , data_offset(tv_offset + offsetof(TypedValue, m_data)) {}
  tv_val_offset_wide(ptrdiff_t type_offset, ptrdiff_t data_offset)
    : type_offset(type_offset)
    , data_offset(data_offset) {}

  ptrdiff_t typeOffset() const { return type_offset; }
  ptrdiff_t dataOffset() const { return data_offset; }

  tv_val_offset_wide shift(ptrdiff_t off) const {
    return {
      type_offset + off,
      data_offset + off
    };
  }

  /* extract a tv_val from a given base address */
  tv_lval apply(char* base) const {
    return tv_lval {
      reinterpret_cast<DataType*>(base + typeOffset()),
      reinterpret_cast<Value*>(base + dataOffset())
    };
  }

  tv_rval apply(const char* base) const {
    return tv_rval {
      reinterpret_cast<const DataType*>(base + typeOffset()),
      reinterpret_cast<const Value*>(base + dataOffset())
     };
  }

  ptrdiff_t type_offset;
  ptrdiff_t data_offset;
};

/* representation of a tv_val_offset when wide mode is off */
struct tv_val_offset_nonwide {
  tv_val_offset_nonwide(ptrdiff_t offset) : offset(offset) {}
  tv_val_offset_nonwide(ptrdiff_t type_offset, ptrdiff_t data_offset)
    : offset(data_offset) {
    static_assert(offsetof(TypedValue, m_data) == 0, "");
    assertx(type_offset == data_offset + offsetof(TypedValue, m_type));
  }

  ptrdiff_t typeOffset() const { return offset + offsetof(TypedValue, m_type); }
  ptrdiff_t dataOffset() const { return offset + offsetof(TypedValue, m_data); }

  /* extract a tv_val from a given base address */
  tv_lval apply(char* base) {
    return reinterpret_cast<TypedValue*>(base + dataOffset());
  }

  tv_rval apply(const char* base) {
    return reinterpret_cast<const TypedValue*>(base + dataOffset());
  }

  tv_val_offset_nonwide shift(ptrdiff_t off) const {
    return {offset + off};
  }

  ptrdiff_t offset;
};

} // namespace detail

using tv_val_offset = std::conditional<wide_tv_val,
                                       detail::tv_val_offset_wide,
                                       detail::tv_val_offset_nonwide>::type;

////////////////////////////////////////////////////////////

// tv_lval-like class used specifically for lvals to frame
// locals. Contains a single pointer to the type, and recomputes the
// pointer to the data on demand. This should only be used for
// super-hot code where the register pressure from using a normal
// tv_lval is too expensive.

struct local_lval {
  INLINE_FLATTEN local_lval() : m_type{nullptr} {}
  // We don't allow implicit conversion because you need to be sure
  // the tv_lval points at a frame local.
  INLINE_FLATTEN explicit local_lval(tv_lval lval)
    : m_type{lval.is_set() ? &lval.type() : nullptr}
  {
    assertx(!m_type || valFromType(m_type) == &lval.val());
  }

  INLINE_FLATTEN bool is_set() const { return m_type; }
  INLINE_FLATTEN explicit operator bool() const { return is_set(); }

  INLINE_FLATTEN DataType& type() const { assertx(is_set()); return *m_type; }
  INLINE_FLATTEN Value& val() const { return *valFromType(&type()); }

  INLINE_FLATTEN tv_lval lval() const { return tv_lval{&type(), &val()}; }
  INLINE_FLATTEN TypedValue tv() const { return TypedValue{val(), type()}; }
  INLINE_FLATTEN TypedValue operator*() const { return tv(); }

  static constexpr size_t typeOff() { return offsetof(local_lval, m_type); }

  static int valOffsetFromTypeOffset(int typeOffset) {
    static_assert(TVOFF(m_type) == sizeof(Value), "");
    return typeOffset - sizeof(Value);
  }
private:
  static Value* valFromType(DataType* t) {
    static_assert(TVOFF(m_type) == sizeof(Value), "");
    return (Value*)t - 1;
  }

  DataType* m_type;
};

INLINE_FLATTEN auto& type(const local_lval& val) { return val.type(); }
INLINE_FLATTEN auto& val(const local_lval& val) { return val.val(); }
INLINE_FLATTEN TypedValue as_tv(const local_lval& val) { return val.tv(); }

}

#include "hphp/runtime/base/tv-val-inl.h"
