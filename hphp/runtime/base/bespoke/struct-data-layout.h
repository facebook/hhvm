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

#include "hphp/util/arch.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/types.h"

namespace HPHP::bespoke {
struct StructDict;
struct StructLayout;
namespace detail_struct_data_layout {

auto constexpr stores_utv = (arch() == Arch::X64 || arch() == Arch::ARM);

template<bool is_const, typename T> using maybe_const_t =
  typename std::conditional<is_const, const T, T>::type;

struct TypePosValLayout {

  template <
    bool is_const,
    typename T = maybe_const_t<is_const, StructDict>
  >
  static tv_val<is_const> tvAtSlot(T* sad, Slot slot) {
    assertx(slot < sad->numFields());
    return {&rawTypes(sad)[slot], &rawValues(sad)[slot]};
  }
  static size_t positionOffset(const StructDict* sad);
  static size_t positionOffset(const StructLayout*);
  static size_t typeOffsetForSlot(const StructLayout*, Slot slot);
  static size_t valueOffsetForSlot(const StructLayout*, Slot slot);

  static size_t numFieldsOffset();
  static size_t numFieldsSize();
  static size_t numFields(const StructDict*);
  static bool isBigStruct(const StructDict*) { return false; }
  static bool isBigStruct(const StructLayout*) { return false; }

  static size_t staticTypeOffset();
  static size_t valueOffsetOffset();
  static constexpr size_t valueOffsetSize() {
    return sizeof(ArrayData::m_extra_hi8);
  }

  static void init(StructDict*);
  static void initSizeIndex(StructLayout*);

  static bool checkInvariants(const StructDict* sad);

  using PosType = uint8_t;
  // Fields used to initialize a new StructDict. The "m_extra_initializer" is
  // computed when we create the layout and used to initialize three fields in
  // the array header in one go in the JIT.
  //
  // The field's layout should pun our usage of ArrayData's m_extra field.
  typedef struct {
    PosType m_num_fields;
    uint8_t m_value_offset_in_values;
    bespoke::LayoutIndex m_layout_index;
  } HeaderData;

private:
  static size_t valueOffset(const StructDict* sad);
  static DataType* rawTypes(StructDict* sad);
  static const DataType* rawTypes(const StructDict* sad);
  static Value* rawValues(StructDict* sad);
  static const Value* rawValues(const StructDict* sad);
  static HeaderData* header(StructLayout*);
};

struct UnalignedTVLayout {

  template <
    bool is_const,
    typename T = maybe_const_t<is_const, StructDict>,
    typename U = maybe_const_t<is_const, UnalignedTypedValue>
  >
  static tv_val<is_const> tvAtSlot(
    T* sad,
    Slot slot) {
    assertx(slot < sad->numFields());
    return reinterpret_cast<U*>(sad + 1) + slot;
  }
  static size_t positionOffset(const StructDict* sad);
  static size_t positionOffset(const StructLayout*);
  static size_t typeOffsetForSlot(const StructLayout*, Slot slot);
  static size_t valueOffsetForSlot(const StructLayout*, Slot slot);

  static size_t numFieldsOffset();
  static size_t numFieldsSize();
  static size_t numFields(const StructDict*);
  static bool isBigStruct(const StructDict*);
  static bool isBigStruct(const StructLayout*);

  static void init(StructDict*);
  static void initSizeIndex(StructLayout*);

  static bool checkInvariants(const StructDict*) {
    return true;
  }

  using PosType = uint16_t;
  // Fields used to initialize a new StructDict. The "m_extra_initializer" is
  // computed when we create the layout and used to initialize three fields in
  // the array header in one go in the JIT.
  //
  // The field's layout should pun our usage of ArrayData's m_extra field.
  typedef struct {
    PosType m_num_fields;
    bespoke::LayoutIndex m_layout_index;
  } HeaderData;

private:
  static HeaderData* header(StructLayout*);
};
}

using StructDataLayout =
  std::conditional_t<detail_struct_data_layout::stores_utv,
  detail_struct_data_layout::UnalignedTVLayout,
  detail_struct_data_layout::TypePosValLayout>;
}
