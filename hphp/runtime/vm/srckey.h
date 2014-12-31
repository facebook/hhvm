/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_SRCKEY_H_
#define incl_HPHP_SRCKEY_H_

#include "hphp/runtime/base/types.h"

#include "hphp/runtime/vm/func.h"

#include <boost/operators.hpp>
#include <tuple>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * A SrcKey is a logical source instruction---it's enough to identify
 * these using a (Func id, hhbc instruction) pair.
 */
struct SrcKey : private boost::totally_ordered<SrcKey> {
  static_assert(sizeof(FuncId) == sizeof(uint32_t), "");
  static_assert(sizeof(Offset) == sizeof(uint32_t), "");
  typedef uint64_t AtomicInt;
  struct Hasher;

  SrcKey()
    : m_funcId(InvalidFuncId)
    , m_offset(0)
    , m_resumed(false)
  {}

  SrcKey(const Func* f, Offset off, bool resumed)
    : m_funcId(f->getFuncId())
    , m_offset((uint32_t)off)
    , m_resumed(resumed)
  {
    assert(0 <= (int32_t)off);
  }

  SrcKey(const Func* f, PC i, bool resumed)
    : m_funcId(f->getFuncId())
    , m_offset((uint32_t)f->unit()->offsetOf(i))
    , m_resumed(resumed)
  {
    assert(0 <= (int32_t)f->unit()->offsetOf(i));
  }

  SrcKey(FuncId funcId, Offset off, bool resumed)
    : m_funcId{funcId}
    , m_offset{(uint32_t)off}
    , m_resumed{resumed}
  {
    assert(0 <= (int32_t)off);
  }

  explicit SrcKey(AtomicInt in)
    : m_atomicInt(in)
  {}

  bool valid() const {
    return m_funcId != InvalidFuncId;
  }

  // Packed representation of SrcKeys for use in contexts where we
  // want atomicity.  (SrcDB.)
  AtomicInt toAtomicInt() const {
    return m_atomicInt;
  }
  static SrcKey fromAtomicInt(AtomicInt in) {
    return SrcKey { in };
  }

  void setFuncId(FuncId id) {
    assert(id != InvalidFuncId);
    m_funcId = id;
  }

  FuncId getFuncId() const {
    assert(m_funcId != InvalidFuncId);
    return m_funcId;
  }

  const Func* func() const {
    return Func::fromFuncId(m_funcId);
  }

  const Unit* unit() const {
    return func()->unit();
  }

  Op op() const {
    return unit()->getOpcode(offset());
  }

  PC pc() const {
    return unit()->at(offset());
  }

  std::string showInst() const;

  void setOffset(Offset o) {
    assert(0 <= (int32_t)o);
    m_offset = (uint32_t)o;
  }

  int offset() const {
    return m_offset;
  }

  bool resumed() const {
    return m_resumed;
  }

  OffsetSet succOffsets() const {
    return instrSuccOffsets((Op*)pc(), unit());
  }

  /*
   * Advance the SrcKey to the next instruction.
   *
   * If the SrcKey points to the last instruction in a function, this
   * will advance past the end of the function, and potentially
   * contain an invalid bytecode offset.
   */
  void advance(const Unit* u = nullptr) {
    m_offset += instrLen((Op*)(u ? u : unit())->at(offset()));
  }

  /*
   * Return a SrcKey representing the next instruction, without
   * mutating this SrcKey.
   */
  SrcKey advanced(const Unit* u = nullptr) const {
    auto tmp = *this;
    tmp.advance(u);
    return tmp;
  }

  bool operator==(const SrcKey& r) const {
    return m_atomicInt == r.m_atomicInt;
  }

  bool operator<(const SrcKey& r) const {
    return m_atomicInt < r.m_atomicInt;
  }

  std::string getSymbol() const;

private:
  union {
    AtomicInt m_atomicInt;
    struct {
      FuncId m_funcId;
      uint32_t m_offset:31;
      bool m_resumed:1;
    };
  };
};

struct SrcKey::Hasher {
  size_t operator()(SrcKey sk) const {
    return hash_int64(sk.toAtomicInt());
  }
};

typedef hphp_hash_set<SrcKey,SrcKey::Hasher> SrcKeySet;

//////////////////////////////////////////////////////////////////////

std::string show(SrcKey sk);
std::string showShort(SrcKey sk);

void sktrace(SrcKey sk, const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);
#define SKTRACE(level, sk, ...) \
  ONTRACE(level, sktrace(sk, __VA_ARGS__))

}

#endif
