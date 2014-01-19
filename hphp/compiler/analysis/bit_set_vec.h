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

#ifndef incl_HPHP_BIT_SET_VEC_H_
#define incl_HPHP_BIT_SET_VEC_H_

#include <limits.h>
#include <stddef.h>
#include "hphp/util/assertions.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class BitSetBlock;
class BitSetRow;

class BitOps {
  public:
  typedef size_t Bits;
  static const size_t elemSize = CHAR_BIT * sizeof(BitOps::Bits);

#define BITOP_DECLARE1(name, op)                                        \
  private:                                                              \
  static void name##_func(Bits *out, Bits *in1) {                       \
    *out = op;                                                          \
  }                                                                     \
  public:                                                               \
  static void name(size_t width, Bits *out, Bits *in1);

#define BITOP_DECLARE2(name, op)                                        \
  private:                                                              \
  static void name##_func(Bits *out, Bits *in1, Bits *in2) {            \
    *out = op;                                                          \
  }                                                                     \
  public:                                                               \
  static void name(size_t width, Bits *out, Bits *in1, Bits *in2);

#define BITOP_DECLARE3(name, op)                                \
  private:                                                      \
  static void name##_func(Bits *out,                            \
                          Bits *in1, Bits *in2, Bits *in3) {    \
    *out = op;                                                  \
  }                                                             \
  public:                                                       \
  static void name(size_t width, Bits *out,                     \
                   Bits *in1, Bits *in2, Bits *in3);

#define BITOP_DEFINE1(name)                                     \
  void BitOps::name(size_t width, Bits *out, Bits *in1) {       \
    bit_op1(name##_func, width, out, in1);                      \
  }

#define BITOP_DEFINE2(name)                     \
  void BitOps::name(size_t width, Bits *out,    \
                    Bits *in1, Bits *in2) {     \
    bit_op2(name##_func, width, out, in1, in2);  \
  }

#define BITOP_DEFINE3(name)                             \
  void BitOps::name(size_t width, Bits *out,            \
                    Bits *in1, Bits *in2, Bits *in3) {  \
    bit_op3(name##_func, width, out, in1, in2, in3);  \
  }

  BITOP_DECLARE1(bit_copy, *in1);
  BITOP_DECLARE1(bit_complement, ~*in1);

  BITOP_DECLARE2(bit_and, *in1 & *in2);
  BITOP_DECLARE2(bit_or, *in1 | *in2);
  BITOP_DECLARE2(bit_xor, *in1 ^ *in2);

  BITOP_DECLARE3(bit_and_and, *in1 & *in2 & *in3);
  BITOP_DECLARE3(bit_or_or, *in1 | *in2 | *in3);
  BITOP_DECLARE3(bit_and_or, (*in1 & *in2) | *in3);
  BITOP_DECLARE3(bit_or_and, (*in1 | *in2) & *in3);
  BITOP_DECLARE3(bit_or_andc, (*in1 | *in2) & ~*in3);
  BITOP_DECLARE3(bit_andc_or, (*in1 & ~*in2) | *in3);

  public:
  static bool bit_equal(size_t width, Bits *in1, Bits *in2);
  static bool get_bit(size_t bit, Bits *bits);
  static void set_bit(size_t bit, Bits *bits, bool value);
  static void set(size_t width, Bits *in, Bits value);

  private:
  template <typename OP>
  static void bit_op1(OP func, size_t width, Bits *out, Bits *in1) {
    width += elemSize - 1;
    while (width >= elemSize) {
      func(out++, in1++);
      width -= elemSize;
    }
  }

  template <typename OP>
  static void bit_op2(OP func, size_t width, Bits *out, Bits *in1, Bits *in2) {
    width += elemSize - 1;
    while (width >= elemSize) {
      func(out++, in1++, in2++);
      width -= elemSize;
    }
  }

  template <typename OP>
  static void bit_op3(OP func, size_t width,
                      Bits *out, Bits *in1, Bits *in2, Bits *in3) {
    width += elemSize - 1;
    while (width >= elemSize) {
      func(out++, in1++, in2++, in3++);
      width -= elemSize;
    }
  }
};

class BitSetVec {
  public:
  static const size_t elemSize = BitOps::elemSize;
  BitSetVec() : m_maxId(0), m_idOffsets(0), m_bits(0) {}
  BitSetVec(int blocks, size_t width, int rows, int *rowIds) {
    alloc(blocks, width, rows, rowIds);
  }
  void alloc(int blocks, size_t width, int rows, int *rowIds);
  ~BitSetVec() { reset(); }

  static BitOps::Bits *add(BitOps::Bits *base, size_t offset) {
    return (BitOps::Bits*)((char*)base + offset);
  }

  BitSetBlock getBlock(int block);
  size_t width() const { return m_width; }
  size_t rowOffset(int row) const {
    always_assert((unsigned)row < (unsigned)m_maxId);
    size_t offset = m_idOffsets[row];
    return offset - 1;
  }
  bool rowExists(int row) const {
    return ((unsigned)row < (unsigned)m_maxId) &&
      m_idOffsets[row];
  }

  BitOps::Bits *getTempBits(int id) const;

  private:
  void reset();

  size_t m_width;
  int m_maxId;
  size_t *m_idOffsets;
  size_t m_rowSize;
  size_t m_blockSize;
  BitOps::Bits *m_bits;
};

class BitSetBlock {
  friend class BitSetVec;
  public:
  BitSetBlock() : m_bits(0), m_owner(0) {}
  BitOps::Bits *getRow(int row) const;
  void setBit(int row, int id, bool v);
  bool getBit(int row, int id) const;

  private:
  BitSetBlock(BitOps::Bits *bits, const BitSetVec &owner) :
  m_bits(bits), m_owner(&owner) {}

  BitOps::Bits *m_bits;
  const BitSetVec *m_owner;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __BIT_SET_VEC_H__
