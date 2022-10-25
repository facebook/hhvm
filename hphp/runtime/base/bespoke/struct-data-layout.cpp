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

#include "hphp/runtime/base/bespoke/struct-data-layout.h"

#include "hphp/runtime/base/bespoke/struct-dict.h"

namespace HPHP::bespoke {
namespace detail_struct_data_layout {
size_t TypePosValLayout::valueOffset(const StructDict* sad) {
  return sad->m_extra_hi8 * sizeof(Value);
}

DataType* TypePosValLayout::rawTypes(StructDict* sad) {
  return reinterpret_cast<DataType*>(
    reinterpret_cast<char*>(sad + 1));
}

const DataType* TypePosValLayout::rawTypes(const StructDict* sad) {
  return rawTypes(const_cast<StructDict*>(sad));
}

Value* TypePosValLayout::rawValues(StructDict* sad) {
  return reinterpret_cast<Value*>(
      reinterpret_cast<char*>(sad) + valueOffset(sad));
}

const Value* TypePosValLayout::rawValues(const StructDict* sad) {
  return rawValues(const_cast<StructDict*>(sad));
}

bool TypePosValLayout::checkInvariants(const StructDict* sad) {
  assertx(valueOffsetForSlot(sad->layout(), 0) == valueOffset(sad));
  return true;
}

void TypePosValLayout::init(StructDict* sad) {
  memset(rawTypes(sad), static_cast<int>(KindOfUninit), sad->numFields());
}

void TypePosValLayout::initSizeIndex(StructLayout* l) {
  static_assert(sizeof(Value) == 8);
  header(l)->m_value_offset_in_values =
    (sizeof(StructDict) + 2 * l->numFields() + 7) / 8;
  auto const bytes =
    (header(l)->m_value_offset_in_values + l->numFields()) * sizeof(Value);
  l->m_size_index = MemoryManager::size2Index(bytes);
}

size_t TypePosValLayout::positionOffset(const StructDict* sad) {
  return sizeof(StructDict) + sad->numFields();
}

size_t TypePosValLayout::positionOffset(const StructLayout* l) {
  return sizeof(StructDict) + l->numFields();
}

size_t TypePosValLayout::typeOffsetForSlot(const StructLayout* l, Slot slot) {
  assertx(slot < l->numFields());
  (void)l;
  return sizeof(StructDict) + slot * sizeof(DataType);
}

size_t TypePosValLayout::valueOffsetForSlot(const StructLayout* l, Slot slot) {
  assertx(slot < l->numFields());
  return
    (header(const_cast<StructLayout*>(l))->m_value_offset_in_values + slot) *
    sizeof(Value);
}

size_t TypePosValLayout::numFieldsOffset() {
  return offsetof(StructDict, m_extra_lo8);
}

size_t TypePosValLayout::numFieldsSize() {
  return sizeof(StructDict::m_extra_lo8);
}

size_t TypePosValLayout::numFields(const StructDict* sad) {
  return sad->m_extra_lo8;
}

size_t TypePosValLayout::staticTypeOffset() {
  return sizeof(StructDict);
}

size_t TypePosValLayout::valueOffsetOffset() {
  return offsetof(StructDict, m_extra_hi8);
}

TypePosValLayout::HeaderData* TypePosValLayout::header(StructLayout* l) {
  return reinterpret_cast<TypePosValLayout::HeaderData*>(&(l->m_header));
}
/////////////////////////////////////////////////////////////////////////////

void UnalignedTVLayout::init(StructDict* sad) {
  memset(sad + 1, static_cast<int>(KindOfUninit),
    sad->numFields() * sizeof(UnalignedTypedValue));
}

void UnalignedTVLayout::initSizeIndex(StructLayout* l) {
  // The actual size can be 1 byte more due to slot alignment,
  // but it should not affect the size index.
  auto const slotSize = l->isBigStruct() ? 2 : 1;
  auto const bytes = sizeof(StructDict) +
    (sizeof(UnalignedTypedValue) + slotSize) * l->numFields();
  l->m_size_index = MemoryManager::size2Index(bytes);
}

size_t UnalignedTVLayout::positionOffset(const StructDict* sad) {
  auto const unalignedOffset =
    sizeof(StructDict) + sad->numFields() * sizeof(UnalignedTypedValue);
  return sad->isBigStruct() ? (unalignedOffset + 1) & ~1 : unalignedOffset;
}

size_t UnalignedTVLayout::positionOffset(const StructLayout* l) {
  auto const unalignedOffset =
    sizeof(StructDict) + l->numFields() * sizeof(UnalignedTypedValue);
  return l->isBigStruct() ? (unalignedOffset + 1) & ~1 : unalignedOffset;
}

size_t UnalignedTVLayout::typeOffsetForSlot(
  UNUSED const StructLayout* l, Slot slot) {
  assertx(slot < l->numFields());
  return sizeof(StructDict) +
    slot * sizeof(UnalignedTypedValue) +
    offsetof(UnalignedTypedValue, m_type);
}

size_t UnalignedTVLayout::valueOffsetForSlot(
  UNUSED const StructLayout* l, Slot slot) {
  assertx(slot < l->numFields());
  return sizeof(StructDict) +
    slot * sizeof(UnalignedTypedValue) +
    offsetof(UnalignedTypedValue, m_data);
}

size_t UnalignedTVLayout::numFieldsOffset() {
  return offsetof(StructDict, m_extra_lo16);
}

size_t UnalignedTVLayout::numFieldsSize() {
  return sizeof(StructDict::m_extra_lo16);
}

size_t UnalignedTVLayout::numFields(const StructDict* sad) {
  return sad->m_extra_lo16;
}

bool UnalignedTVLayout::isBigStruct(const StructDict* sad) {
  return sad->numFields() > 0xff;
}

bool UnalignedTVLayout::isBigStruct(const StructLayout* l) {
  return l->numFields() > 0xff;
}

UnalignedTVLayout::HeaderData* UnalignedTVLayout::header(StructLayout* l) {
  return reinterpret_cast<UnalignedTVLayout::HeaderData*>(&l->m_header);
}
}
}
