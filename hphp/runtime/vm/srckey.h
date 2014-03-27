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
  typedef uint64_t AtomicInt;
  struct Hasher;

  SrcKey()
    : m_funcId(InvalidFuncId)
    , m_offset(0)
  {}

  SrcKey(const Func* f, Offset off)
    : m_funcId(f->getFuncId())
    , m_offset(off)
  {}

  SrcKey(const Func* f, PC i)
    : m_funcId(f->getFuncId())
    , m_offset(f->unit()->offsetOf(i))
  {}

  SrcKey(FuncId funcId, Offset off)
    : m_funcId{funcId}
    , m_offset{off}
  {}

  bool valid() const {
    return m_funcId != InvalidFuncId;
  }

  // Packed representation of SrcKeys for use in contexts where we
  // want atomicity.  (SrcDB.)
  AtomicInt toAtomicInt() const {
    return uint64_t(getFuncId()) << 32 | uint64_t(offset());
  }
  static SrcKey fromAtomicInt(AtomicInt in) {
    return SrcKey { uint32_t(in >> 32), (Offset) int32_t(in & 0xffffffff) };
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
    m_offset = o;
  }

  int offset() const {
    return m_offset;
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
    return offset() == r.offset() &&
           getFuncId() == r.getFuncId();
  }

  bool operator<(const SrcKey& r) const {
    return std::make_tuple(getFuncId(), offset()) <
           std::make_tuple(r.getFuncId(), r.offset());
  }

  std::string getSymbol() const;

private:
  FuncId m_funcId;
  Offset m_offset;
};

struct SrcKey::Hasher {
  size_t operator()(SrcKey sk) const {
    return hash_int64_pair(sk.getFuncId(), uint64_t(sk.offset()));
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
