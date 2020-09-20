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

#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/resumable.h"

#include <boost/operators.hpp>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

namespace arrprov { struct Tag; }

struct Func;
struct Unit;

///////////////////////////////////////////////////////////////////////////////

/*
 * A SrcKey is a logical source instruction---it's enough to identify
 * these using a (Func id, hhbc instruction) pair.
 */
struct SrcKey : private boost::totally_ordered<SrcKey> {
  static_assert(sizeof(FuncId) == sizeof(uint32_t), "");
  static_assert(sizeof(Offset) == sizeof(uint32_t), "");

  /////////////////////////////////////////////////////////////////////////////
  // Types.

  using AtomicInt = uint64_t;

  struct Hasher;
  struct TbbHashCompare;

  /*
   * Used for SrcKeys corresponding to the prologue which precedes a function
   * entry source location.
   *
   * Used disjointly from the `resumeMode'.
   */
  enum class PrologueTag {};

  /////////////////////////////////////////////////////////////////////////////

  /*
   * Invalid SrcKey constructor.
   */
  SrcKey();

  SrcKey(const Func* f, Offset off, ResumeMode resumeMode);
  SrcKey(const Func* f, PC pc, ResumeMode resumeMode);
  SrcKey(FuncId funcId, Offset off, ResumeMode resumeMode);

  SrcKey(const Func* f, Offset off, PrologueTag);
  SrcKey(const Func* f, PC pc, PrologueTag);
  SrcKey(FuncId funcId, Offset off, PrologueTag);

  SrcKey(SrcKey other, Offset off);

  explicit SrcKey(AtomicInt in);
  static SrcKey fromAtomicInt(AtomicInt in);

  /////////////////////////////////////////////////////////////////////////////

  /*
   * Whether the SrcKey has a valid FuncId.
   *
   * Does not check for validity of any other fields.
   */
  bool valid() const;

  /*
   * Packed representation of SrcKeys for use in contexts (e.g. the SrcDB)
   * where we want atomicity.
   */
  AtomicInt toAtomicInt() const;

  /*
   * Direct accessors.
   */
  FuncId funcID() const;
  int offset() const;
  bool prologue() const;
  ResumeMode resumeMode() const;
  bool hasThis() const;

  /*
   * Derived accessors.
   */
  const Func* func() const;
  const Unit* unit() const;
  Op op() const;
  PC pc() const;

  /////////////////////////////////////////////////////////////////////////////

  /*
   * Set this SrcKey's offset.
   *
   * Does not perform any bounds or consistency checks.
   */
  void setOffset(Offset o);

  /*
   * Get all possible Offsets for the next bytecode.
   */
  OffsetSet succOffsets() const;

  /*
   * Advance the SrcKey to the next instruction.
   *
   * If the SrcKey points to the last instruction in a function, this will
   * advance past the end of the function, and potentially contain an invalid
   * bytecode offset.
   */
  void advance(const Func* f = nullptr);

  /*
   * Return a SrcKey representing the next instruction, without mutating this
   * SrcKey.
   */
  SrcKey advanced(const Func* f = nullptr) const;

  /////////////////////////////////////////////////////////////////////////////

  /*
   * Comparisons.
   *
   * SrcKeys are ordered by their AtomicInt values.
   */
  bool operator==(const SrcKey& r) const;
  bool operator<(const SrcKey& r) const;

  /*
   * Stringification.
   */
  std::string showInst() const;
  std::string getSymbol() const;

  /////////////////////////////////////////////////////////////////////////////

private:
  uint32_t encodeResumeMode(ResumeMode resumeMode);
  uint32_t encodePrologue();

  /////////////////////////////////////////////////////////////////////////////

  union {
    AtomicInt m_atomicInt;
    struct {
      FuncId m_funcID;
      uint32_t m_offset : 30;
      uint32_t m_resumeModeAndPrologue : 2;
    };
  };
};

///////////////////////////////////////////////////////////////////////////////

struct SrcKey::Hasher {
  size_t operator()(SrcKey sk) const {
    return hash_int64(sk.toAtomicInt());
  }
};

using SrcKeySet = hphp_hash_set<SrcKey,SrcKey::Hasher>;

///////////////////////////////////////////////////////////////////////////////

struct SrcKey::TbbHashCompare {
  static size_t hash(const SrcKey& sk) { return hash_int64(sk.toAtomicInt()); }
  static bool equal(const SrcKey& a, const SrcKey& b) { return a == b; }
};

///////////////////////////////////////////////////////////////////////////////

std::string show(SrcKey sk);
std::string showShort(SrcKey sk);

void sktrace(SrcKey sk, ATTRIBUTE_PRINTF_STRING const char *fmt, ...)
  ATTRIBUTE_PRINTF(2,3);
#define SKTRACE(level, sk, ...) \
  ONTRACE(level, sktrace(sk, __VA_ARGS__))

///////////////////////////////////////////////////////////////////////////////

}

#include "hphp/runtime/vm/srckey-inl.h"

