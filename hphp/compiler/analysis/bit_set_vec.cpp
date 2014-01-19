/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include <stdlib.h>
#include "hphp/compiler/analysis/bit_set_vec.h"
#include "hphp/compiler/analysis/data_flow.h"

using namespace HPHP;

BITOP_DEFINE1(bit_copy);
BITOP_DEFINE1(bit_complement);
BITOP_DEFINE2(bit_and)
BITOP_DEFINE2(bit_or)
BITOP_DEFINE2(bit_xor)
BITOP_DEFINE3(bit_and_and)
BITOP_DEFINE3(bit_or_or)
BITOP_DEFINE3(bit_and_or)
BITOP_DEFINE3(bit_or_and)
BITOP_DEFINE3(bit_or_andc)
BITOP_DEFINE3(bit_andc_or)

void BitSetVec::alloc(int blocks, size_t width, int rows, int *rowIds) {
  reset();

  m_width = width;
  m_maxId = 0;

  bool noTranslate = rows < 0;
  if (noTranslate) {
    m_maxId = rows = -rows;
  } else {
    for (int i = 0; i < rows; i++) {
      int rid = rowIds[i];
      if (rid >= m_maxId) m_maxId = rid + 1;
    }
  }

  m_rowSize = (width + elemSize - 1) / elemSize * sizeof(BitOps::Bits);
  m_idOffsets = (size_t*)calloc(m_maxId, sizeof(size_t));
  for (int i = 0; i < rows; i++) {
    int rid = noTranslate ? i : rowIds[i];
    m_idOffsets[rid] = i * m_rowSize + 1;
  }
  m_blockSize = m_rowSize * rows;
  m_bits = (BitOps::Bits*)calloc(m_blockSize, blocks);
  if (!noTranslate) {
    size_t offset = 0;
    for (int b = 0; b < blocks; b++) {
      for (int i = 0; i < rows; i++) {
        if (DataFlow::GetInit(rowIds[i])) {
          memset(add(m_bits, offset), 255, m_rowSize);
        }
        offset += m_rowSize;
      }
    }
  }
}

void BitSetVec::reset() {
  free(m_idOffsets);
  free(m_bits);
}

BitSetBlock BitSetVec::getBlock(int block) {
  return BitSetBlock(add(m_bits, block * m_blockSize), *this);
}

BitOps::Bits *BitSetVec::getTempBits(int id) const {
  size_t offset = id * m_rowSize;
  always_assert(offset < m_blockSize);

  return add(m_bits, offset);
}

BitOps::Bits *BitSetBlock::getRow(int row) const {
  size_t offset = m_owner->rowOffset(row);
  always_assert(offset != (size_t)-1);
  return BitSetVec::add(m_bits, offset);
}

void BitSetBlock::setBit(int row, int id, bool v) {
  BitOps::Bits *bits = getRow(row);
  always_assert((unsigned)id < m_owner->width());
  BitOps::set_bit(id, bits, v);
}

bool BitSetBlock::getBit(int row, int id) const {
  BitOps::Bits *bits = getRow(row);
  always_assert((unsigned)id < m_owner->width());
  return BitOps::get_bit(id, bits);
}

bool BitOps::bit_equal(size_t width, Bits *in1, Bits *in2) {
  while (width >= BitSetVec::elemSize) {
    if (*in1++ != *in2++) return false;
    width -= BitSetVec::elemSize;
  }
  if (!width) return true;
  Bits mask = ~((Bits)-1 << width);
  return (*in1 & mask) == (*in2 & mask);
}

void BitOps::set(size_t width, Bits *out, Bits value) {
  width += elemSize - 1;
  while (width >= elemSize) {
    *out++ = value;
    width -= elemSize;
  }
}

bool BitOps::get_bit(size_t bit, Bits *bits) {
  size_t offset = bit / BitSetVec::elemSize;
  int b = bit % BitSetVec::elemSize;
  return (bits[offset] >> b) & 1;
}

void BitOps::set_bit(size_t bit, Bits *bits, bool value) {
  size_t offset = bit / BitSetVec::elemSize;
  int b = bit % BitSetVec::elemSize;
  Bits m = ~((Bits)1 << b);
  Bits v = (Bits)value << b;
  bits[offset] = (bits[offset] & m) | v;
}

