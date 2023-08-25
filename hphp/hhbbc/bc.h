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

#include <vector>
#include <utility>
#include <type_traits>

#include <algorithm>

#include <folly/Hash.h>

#include "hphp/util/compact-vector.h"
#include "hphp/runtime/vm/hhbc.h"

#include "hphp/hhbbc/src-loc.h"
#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/type-system.h"

namespace HPHP { namespace HHBBC {

struct Bytecode;

//////////////////////////////////////////////////////////////////////

/*
 * The following creates a struct for each bytecode using the opcode
 * table.  Each opcode will be named bc::opcode, and has a single
 * constructor that takes its immediate types in order.
 *
 * E.g.
 *
 *   auto pushInt = bc::Int { 2 };  // Push literal int
 */

//////////////////////////////////////////////////////////////////////

struct NamedLocal {
  NamedLocal()
    : name(kInvalidLocalName)
    , id(NoLocalId)
  {}
  NamedLocal(LocalName name, LocalId id)
    : name(name)
    , id(id)
  {}
  /* implicit */ NamedLocal(const ::HPHP::NamedLocal& nl)
    : name(nl.name)
    , id(nl.id)
  {}
  /* implicit */ operator auto(){
    return ::HPHP::NamedLocal{name, static_cast<int32_t>(id)};
  }
  LocalName name;
  LocalId id;

  template <typename SerDe> void serde(SerDe& sd) {
    sd(name)(id);
  }
};

inline bool operator==(NamedLocal a, NamedLocal b) {
  return a.name == b.name && a.id == b.id;
}

inline bool operator!=(NamedLocal a, NamedLocal b) {
  return !(a == b);
}

struct MKey {
  MKey()
    : mcode{MW}
    , rop{ReadonlyOp::Any}
    , int64{0}
  {}

  MKey(MemberCode mcode, NamedLocal local, ReadonlyOp rop)
    : mcode{mcode}
    , rop{rop}
    , local{local}
  {}

  MKey(MemberCode mcode, int32_t idx, ReadonlyOp rop)
    : mcode{mcode}
    , rop{rop}
    , idx{idx}
  {}

  MKey(MemberCode mcode, int64_t int64, ReadonlyOp rop)
    : mcode{mcode}
    , rop{rop}
    , int64{int64}
  {}

  MKey(MemberCode mcode, SString litstr, ReadonlyOp rop)
    : mcode{mcode}
    , rop{rop}
    , litstr{litstr}
  {}

  MemberCode mcode;
  ReadonlyOp rop;
  union {
    SString litstr;
    int64_t int64;
    int64_t idx;
    NamedLocal local;
  };

  template <typename SerDe> void serde(SerDe& sd) {
    sd(mcode)(rop);
    switch (mcode) {
      case MEL: case MPL:           sd(local);  break;
      case MEC: case MPC:           sd(idx);    break;
      case MET: case MPT: case MQT: sd(litstr); break;
      case MEI:                     sd(int64);  break;
      case MW:                                  break;
    }
  }
};

inline bool operator==(MKey a, MKey b) {
  return a.mcode == b.mcode && a.int64 == b.int64 && a.rop == b.rop;
}

inline bool operator!=(MKey a, MKey b) {
  return !(a == b);
}

// A contiguous range of locals. The count is the number of locals including the
// first. If the range is empty, count will be zero and first's value is
// arbitrary.
struct LocalRange {
  LocalId  first;
  uint32_t count;
  template <typename SerDe> void serde(SerDe& sd) {
    sd(first)(count);
  }
};

inline bool operator==(const LocalRange& a, const LocalRange& b) {
  return a.first == b.first && a.count == b.count;
}

inline bool operator!=(const LocalRange& a, const LocalRange& b) {
  return !(a == b);
}

struct FCallArgsLong : FCallArgsBase {
  explicit FCallArgsLong(Flags flags, uint32_t numArgs, uint32_t numRets,
                         std::unique_ptr<uint8_t[]> inoutArgs,
                         std::unique_ptr<uint8_t[]> readonlyArgs,
                         BlockId asyncEagerTarget, LSString context)
    : FCallArgsBase(flags, numArgs, numRets)
    , inoutArgs(std::move(inoutArgs))
    , readonlyArgs(std::move(readonlyArgs))
    , asyncEagerTarget(asyncEagerTarget)
    , context(context) {}
  explicit FCallArgsLong(FCallArgsBase base,
                         std::unique_ptr<uint8_t[]> inoutArgs,
                         std::unique_ptr<uint8_t[]> readonlyArgs,
                         BlockId asyncEagerTarget, LSString context)
    : FCallArgsBase(std::move(base))
    , inoutArgs(std::move(inoutArgs))
    , readonlyArgs(std::move(readonlyArgs))
    , asyncEagerTarget(asyncEagerTarget)
    , context(context) {}

  FCallArgsLong(const FCallArgsLong& o)
    : FCallArgsLong(o.flags, o.numArgs, o.numRets, nullptr, nullptr,
                    o.asyncEagerTarget, o.context) {
    if (o.inoutArgs) {
      auto const numBytes = (numArgs + 7) / 8;
      inoutArgs = std::make_unique<uint8_t[]>(numBytes);
      memcpy(inoutArgs.get(), o.inoutArgs.get(), numBytes);
    }
    if (o.readonlyArgs) {
      auto const numBytes = (numArgs + 7) / 8;
      readonlyArgs = std::make_unique<uint8_t[]>(numBytes);
      memcpy(readonlyArgs.get(), o.readonlyArgs.get(), numBytes);
    }
  }

  bool enforceInOut() const { return inoutArgs.get() != nullptr; }
  bool isInOut(uint32_t i) const {
    assertx(enforceInOut());
    return inoutArgs[i / 8] & (1 << (i % 8));
  }
  bool enforceReadonly() const { return readonlyArgs.get() != nullptr; }
  bool isReadonly(uint32_t i) const {
    assertx(enforceReadonly());
    return readonlyArgs[i / 8] & (1 << (i % 8));
  }

  FCallArgsLong withoutGenerics() const {
    auto fca = *this;
    fca.flags = static_cast<Flags>(fca.flags & ~Flags::HasGenerics);
    return fca;
  }
  FCallArgsLong withoutLockWhileUnwinding() const {
    auto fca = *this;
    fca.flags = static_cast<Flags>(fca.flags & ~Flags::LockWhileUnwinding);
    return fca;
  }
  FCallArgsLong withoutEnforceMutableReturn() const {
    auto fca = *this;
    fca.flags = static_cast<Flags>(fca.flags & ~Flags::EnforceMutableReturn);
    return fca;
  }
  FCallArgsLong withoutEnforceReadonlyThis() const {
    auto fca = *this;
    fca.flags = static_cast<Flags>(fca.flags & ~Flags::EnforceReadonlyThis);
    return fca;
  }

  FCallArgsLong withoutRepack() const {
    auto fca = *this;
    fca.flags = static_cast<Flags>(fca.flags | Flags::SkipRepack);
    return fca;
  }

  FCallArgsLong withoutCoeffectsCheck() const {
    auto fca = *this;
    fca.flags = static_cast<Flags>(fca.flags | Flags::SkipCoeffectsCheck);
    return fca;
  }

  FCallArgsLong withoutInOut() const {
    auto fca = *this;
    fca.inoutArgs = nullptr;
    return fca;
  }

  FCallArgsLong withoutReadonly() const {
    auto fca = *this;
    fca.readonlyArgs = nullptr;
    return fca;
  }

  FCallArgsLong withoutAsyncEagerTarget() const {
    auto fca = *this;
    fca.asyncEagerTarget = NoBlockId;
    return fca;
  }
  template<typename F>
  void applyIO(F f) const {
    f((numArgs + 7) / 8, inoutArgs.get());
  }
  template<typename F>
  void applyReadonly(F f) const {
    f((numArgs + 7) / 8, readonlyArgs.get());
  }
  friend bool operator==(const FCallArgsLong& a, const FCallArgsLong& b) {
    auto const eq = [&] (uint8_t* a, uint8_t* b, uint32_t bytes) {
      if (a == nullptr && b == nullptr) return true;
      if (a == nullptr || b == nullptr) return false;
      return memcmp(a, b, bytes) == 0;
    };

    return
      a.flags == b.flags && a.numArgs == b.numArgs && a.numRets == b.numRets &&
      eq(a.inoutArgs.get(), b.inoutArgs.get(), (a.numArgs + 7) / 8) &&
      eq(a.readonlyArgs.get(), b.readonlyArgs.get(), (a.numArgs + 7) / 8) &&
      a.asyncEagerTarget == b.asyncEagerTarget &&
      a.context == b.context;
  }

  size_t hash() const {
    uint64_t hash = HPHP::hash_int64_pair(numArgs, numRets);
    hash = HPHP::hash_int64_pair(hash, flags);
    if (auto const br = reinterpret_cast<const char*>(inoutArgs.get())) {
      auto const hash_br = hash_string_cs(br, (numArgs + 7) / 8);
      hash = HPHP::hash_int64_pair(hash, hash_br);
    }
    if (auto const br = reinterpret_cast<const char*>(readonlyArgs.get())) {
      auto const hash_br = hash_string_cs(br, (numArgs + 7) / 8);
      hash = HPHP::hash_int64_pair(hash, hash_br);
    }
    hash = HPHP::hash_int64_pair(hash, asyncEagerTarget);
    if (context) hash = HPHP::hash_int64_pair(hash, context->hash());
    return static_cast<size_t>(hash);
  }
  template<int nin>
  uint32_t numPop() const {
    return nin + numInputs() + 1 + numRets;
  }
  template<int nin, int nobj>
  Flavor popFlavor(uint32_t i) const {
    assertx(i < this->template numPop<nin>());
    if (i < nin) return Flavor::C;
    i -= nin;
    if (hasGenerics()) {
      if (i == 0) return Flavor::C;
      i--;
    }
    if (hasUnpack()) {
      if (i == 0) return Flavor::C;
      i--;
    }
    if (i < numArgs) return Flavor::C;
    i -= numArgs;
    if (i == 1 && nobj) return Flavor::C;
    return Flavor::U;
  }
  FCallArgsBase base() const { return *this; }

  std::unique_ptr<uint8_t[]> inoutArgs;
  std::unique_ptr<uint8_t[]> readonlyArgs;
  BlockId asyncEagerTarget;
  LSString context;

  template <typename SerDe>
  static copy_ptr<FCallArgsLong> makeForSerde(SerDe& sd) {
    static_assert(SerDe::deserializing);

    auto base = sd.template make<FCallArgsBase>();
    BlockId asyncEagerTarget;
    LSString context;
    sd(asyncEagerTarget)(context);

    bool hasInout;
    bool hasReadonly;
    sd(hasInout)(hasReadonly);

    auto const numBytes = (base.numArgs + 7) / 8;
    std::unique_ptr<uint8_t[]> inoutArgs;
    std::unique_ptr<uint8_t[]> readonlyArgs;

    if (hasInout) {
      inoutArgs = std::make_unique<uint8_t[]>(numBytes);
      assertx(sd.remaining() >= numBytes);
      std::memcpy(inoutArgs.get(), sd.data(), numBytes);
      sd.advance(numBytes);
    }

    if (hasReadonly) {
      readonlyArgs = std::make_unique<uint8_t[]>(numBytes);
      assertx(sd.remaining() >= numBytes);
      std::memcpy(readonlyArgs.get(), sd.data(), numBytes);
      sd.advance(numBytes);
    }

    return copy_ptr<FCallArgsLong>{
      std::move(base),
      std::move(inoutArgs),
      std::move(readonlyArgs),
      asyncEagerTarget,
      context
    };
  }

  template <typename SerDe> void serde(SerDe& sd) {
    static_assert(!SerDe::deserializing);
    sd(static_cast<const FCallArgsBase&>(*this))
      (asyncEagerTarget)
      (context)
      ((bool)inoutArgs)
      ((bool)readonlyArgs);
    auto const numBytes = (numArgs + 7) / 8;
    if (inoutArgs)    sd.writeRaw((const char*)inoutArgs.get(), numBytes);
    if (readonlyArgs) sd.writeRaw((const char*)readonlyArgs.get(), numBytes);
  }
};

struct FCallArgs {
  using Flags = FCallArgsBase::Flags;
  explicit FCallArgs(uint32_t numArgs)
    : FCallArgs(Flags::FCANone, numArgs, 1, nullptr, nullptr,
                NoBlockId, nullptr) {}
  FCallArgs(Flags flags, uint32_t numArgs, uint32_t numRets,
            std::unique_ptr<uint8_t[]> inoutArgs,
            std::unique_ptr<uint8_t[]> readonlyArgs,
            BlockId asyncEagerTarget,
            LSString context)
    : l{flags, numArgs, numRets,
        std::move(inoutArgs), std::move(readonlyArgs),
        asyncEagerTarget, context} {}
  FCallArgs(const FCallArgs&) = default;
  FCallArgs(FCallArgs&&) = default;

  FCallArgs(const FCallArgsLong& o) : l{o} {}

  bool enforceInOut() const {
    return l->enforceInOut();
  }
  bool isInOut(uint32_t i) const {
    assertx(enforceInOut());
    return l->isInOut(i);
  }
  bool enforceReadonly() const {
    return l->enforceReadonly();
  }
  bool isReadonly(uint32_t i) const {
    assertx(enforceReadonly());
    return l->isReadonly(i);
  }

  FCallArgs withoutGenerics() const {
    return l->withoutGenerics();
  }
  FCallArgs withoutEnforceMutableReturn() const {
    return l->withoutEnforceMutableReturn();
  }
  FCallArgs withoutEnforceReadonlyThis() const {
    return l->withoutEnforceReadonlyThis();
  }

  FCallArgs withoutInOut() const {
    return l->withoutInOut();
  }
  FCallArgs withoutReadonly() const {
    return l->withoutReadonly();
  }
  FCallArgs withoutLockWhileUnwinding() const {
    return l->withoutLockWhileUnwinding();
  }
  FCallArgs withoutAsyncEagerTarget() const {
    return l->withoutAsyncEagerTarget();
  }
  FCallArgs withoutRepack() const {
    return l->withoutRepack();
  }
  FCallArgs withoutCoeffectsCheck() const {
    return l->withoutCoeffectsCheck();
  }

  FCallArgsBase base() const {
    return l->base();
  }

  friend bool operator==(const FCallArgs& a, const FCallArgs& b) {
    if (a.l.get() == b.l.get()) return true;
    return *a.l == *b.l;
  }

  size_t hash() const {
    return l->hash();
  }

  uint32_t numArgs() const {
    return l->numArgs;
  }

  uint32_t numRets() const {
    return l->numRets;
  }

  const uint8_t* inoutArgs() const {
    return l->inoutArgs.get();
  }
  template<typename F>
  void applyIO(F f) const {
    assertx(enforceInOut());
    l->applyIO(f);
  }

  const uint8_t* readonlyArgs() const {
    return l->readonlyArgs.get();
  }
  template<typename F>
  void applyReadonly(F f) const {
    assertx(enforceReadonly());
    l->applyReadonly(f);
  }

  BlockId asyncEagerTarget() const {
    return l->asyncEagerTarget;
  }
  BlockId& asyncEagerTarget() {
    return l.mutate()->asyncEagerTarget;
  }
  bool hasUnpack() const {
    return l->hasUnpack();
  }
  bool hasGenerics() const {
    return l->hasGenerics();
  }
  bool enforceMutableReturn() const {
    return l->enforceMutableReturn();
  }
  bool enforceReadonlyThis() const {
    return l->enforceReadonlyThis();
  }
  bool lockWhileUnwinding() const {
    return l->lockWhileUnwinding();
  }
  uint32_t numInputs() const {
    return l->numInputs();
  }
  bool skipRepack() const {
    return l->skipRepack();
  }
  bool skipCoeffectsCheck() const {
    return l->skipCoeffectsCheck();
  }
  template<int nin>
  uint32_t numPop() const {
    return l->template numPop<nin>();
  }
  template<int nin, int nobj>
  Flavor popFlavor(uint32_t i) const {
    return l->template popFlavor<nin,nobj>(i);
  }
  LSString context() const { return l->context; }

  template <typename SerDe> static FCallArgs makeForSerde(SerDe& sd) {
    static_assert(SerDe::deserializing);
    return FCallArgs{ FCallArgsLong::makeForSerde(sd) };
  }

  template <typename SerDe> void serde(SerDe& sd) {
    assertx(l);
    sd(*l);
  }

private:
  explicit FCallArgs(copy_ptr<FCallArgsLong> l) : l{std::move(l)} {}
  copy_ptr<FCallArgsLong> l;
};

inline bool operator!=(const FCallArgs& a, const FCallArgs& b) {
  return !(a == b);
}

using SwitchTab     = CompactVector<BlockId>;

// The final entry in the SSwitchTab is the default case, it will
// always have a nullptr for the string.
using SSwitchTabEnt = std::pair<LSString,BlockId>;
using SSwitchTab    = CompactVector<SSwitchTabEnt>;

//////////////////////////////////////////////////////////////////////

namespace bc {

//////////////////////////////////////////////////////////////////////

namespace detail {

/*
 * Trivial hasher overrides, which will cause bytecodes to hash according to
 * the hashes defined by hasher_impl.
 */
struct hasher_default {};

/*
 * Default bytecode immediate hashers.
 */
struct hasher_impl {
  static size_t hash(LSString s) { return s->hash(); }
  static size_t hash(RepoAuthType rat) { return rat.hash(); }

  static size_t hash(MKey mkey) {
    auto hash = HPHP::hash_int64_pair(mkey.mcode, mkey.int64);
    return HPHP::hash_int64_pair(hash, (uint64_t)mkey.rop);
  }

  template<class T>
  static size_t hash(const CompactVector<T>& v) {
    return v.empty() ? 0 : v.size() ^ hash(v.front());
  }

  static size_t hash(std::pair<LSString,BlockId> kv) {
    return HPHP::hash_int64_pair(kv.first->hash(), kv.second);
  }

  static size_t hash(NamedLocal loc) {
    return HPHP::hash_int64((((uint64_t)loc.name) << 32) | loc.id);
  }

  static size_t hash(LocalRange range) {
    return HPHP::hash_int64_pair(range.first, range.count);
  }

  static size_t hash(IterArgs ita) {
    auto hash = HPHP::hash_int64_pair(ita.flags, ita.iterId);
    hash = HPHP::hash_int64_pair(hash, ita.keyId);
    return HPHP::hash_int64_pair(hash, ita.valId);
  }

  static size_t hash(FCallArgs fca) {
    return fca.hash();
  }

  template<class T>
  static typename std::enable_if<
    std::is_enum<T>::value,
    size_t
  >::type hash(T t) {
    using U = typename std::underlying_type<T>::type;
    return std::hash<U>()(static_cast<U>(t));
  }

  template<class T>
  static typename std::enable_if<
    !std::is_enum<T>::value,
    size_t
  >::type hash(const T& t) { return std::hash<T>()(t); }
};

/*
 * Hash operand wrapper.
 */
template<typename T, typename S>
struct hash_operand { const T& val; S type; };

// this template isn't really needed. its a workaround for T44007494
template<typename S> struct hash_operand<void*, S> { void* const val; S type; };

/*
 * Hash T using H::operator() if it is compatible, else fall back to
 * hasher_impl (e.g., if H := hasher_default).
 */
template<typename T, typename S, class H>
auto hash(const hash_operand<T,S>& t, H h) -> decltype(h(t.val)) {
  return h(t.val);
}
template<typename T, typename S, class H>
auto hash(const hash_operand<T,S>& t, H h) -> decltype(h(t.val, t.type)) {
  return h(t.val, t.type);
}
template<typename T, typename S, typename... Unused>
typename std::enable_if<sizeof...(Unused) == 1, size_t>::type
hash(const hash_operand<T,S>& t, Unused...) {
  return hasher_impl::hash(t.val);
}

/*
 * Clone of folly::hash::hash_combine_generic.
 */
template <class H>
size_t hash_combine(H /*h*/) {
  return 0;
}

template<class H, typename T, typename... Ts>
size_t hash_combine(H h, const T& t, const Ts&... ts) {
  auto const seed = size_t{hash(t, h)};
  if (sizeof...(ts) == 0) return seed;

  auto const remainder = hash_combine(h, ts...);
  return static_cast<size_t>(folly::hash::hash_128_to_64(seed, remainder));
}

/*
 * Trivial equality overrides, which will cause bytecodes to compare via
 * operator==() on the various immediate types.
 */
struct eq_default {};

/*
 * Equality operand wrapper.
 */
template<typename T, typename S>
struct eq_operand { const T& l; const T& r; S type; };

// this template isn't really needed. its a workaround for T44007494
template<typename S> struct eq_operand<void*, S> {
  void* const l; void* const r; S type;
};

/*
 * Compare two values, using E::operator() if it exists, else the default
 * operator==.
 */
template<typename T, typename S, class E>
auto equals(const eq_operand<T,S>& t, E e) -> decltype(e(t.l, t.r)) {
  return e(t.l, t.r);
}
template<typename T, typename S, class E>
auto equals(const eq_operand<T,S>& t, E e) -> decltype(e(t.l, t.r, t.type)) {
  return e(t.l, t.r, t.type);
}
template<typename T, typename S, typename... Unused>
typename std::enable_if<sizeof...(Unused) == 1, bool>::type
equals(const eq_operand<T,S>& t, Unused...) {
  if constexpr (std::is_floating_point_v<T>) {
    return double_equals(t.l, t.r);
  }
  return t.l == t.r;
}

/*
 * Check if a list of eq_operands are pairwise-equal.
 */
template <class E>
bool eq_pairs(E /*e*/) {
  return true;
}

template<class E, typename T, typename... Ts>
bool eq_pairs(E e, const T& t, const Ts&... ts) {
  return equals(t, e) && (sizeof...(ts) == 0 || eq_pairs(e, ts...));
}

}

//////////////////////////////////////////////////////////////////////

/*
 * Bytecode immediate type tags.
 */
namespace imm {
#define ARGTYPE(name, type)     enum class name : uint8_t {};
#define ARGTYPEVEC(name, type)  enum class name : uint8_t {};
  ARGTYPES
#undef ARGTYPE
#undef ARGTYPEVEC
}

#define IMM_ID_BLA      BLA
#define IMM_ID_SLA      SLA
#define IMM_ID_IVA      IVA
#define IMM_ID_I64A     I64A
#define IMM_ID_LA       LA
#define IMM_ID_NLA      NLA
#define IMM_ID_ILA      ILA
#define IMM_ID_IA       IA
#define IMM_ID_DA       DA
#define IMM_ID_SA       SA
#define IMM_ID_RATA     RATA
#define IMM_ID_AA       AA
#define IMM_ID_BA       BA
#define IMM_ID_OA(type) OA
#define IMM_ID_VSA      VSA
#define IMM_ID_KA       KA
#define IMM_ID_LAR      LAR
#define IMM_ID_ITA      ITA
#define IMM_ID_FCA      FCA

#define IMM_TY_BLA      SwitchTab
#define IMM_TY_SLA      SSwitchTab
#define IMM_TY_IVA      uint32_t
#define IMM_TY_I64A     int64_t
#define IMM_TY_LA       LocalId
#define IMM_TY_NLA      NamedLocal
#define IMM_TY_ILA      LocalId
#define IMM_TY_IA       IterId
#define IMM_TY_DA       double
#define IMM_TY_SA       LSString
#define IMM_TY_RATA     RepoAuthType
#define IMM_TY_AA       SArray
#define IMM_TY_BA       BlockId
#define IMM_TY_OA(type) type
#define IMM_TY_VSA      CompactVector<LSString>
#define IMM_TY_KA       MKey
#define IMM_TY_LAR      LocalRange
#define IMM_TY_ITA      IterArgs
#define IMM_TY_FCA      FCallArgs

#define IMM_NAME_BLA(n)     targets
#define IMM_NAME_SLA(n)     targets
#define IMM_NAME_IVA(n)     arg##n
#define IMM_NAME_I64A(n)    arg##n
#define IMM_NAME_LA(n)      loc##n
#define IMM_NAME_NLA(n)     nloc##n
#define IMM_NAME_ILA(n)     loc##n
#define IMM_NAME_IA(n)      iter##n
#define IMM_NAME_DA(n)      dbl##n
#define IMM_NAME_SA(n)      str##n
#define IMM_NAME_RATA(n)    rat
#define IMM_NAME_AA(n)      arr##n
#define IMM_NAME_BA(n)      target##n
#define IMM_NAME_OA_IMPL(n) subop##n
#define IMM_NAME_OA(type)   IMM_NAME_OA_IMPL
#define IMM_NAME_VSA(n)     keys
#define IMM_NAME_KA(n)      mkey
#define IMM_NAME_LAR(n)     locrange
#define IMM_NAME_ITA(n)     ita
#define IMM_NAME_FCA(n)     fca

#define IMM_TARGETS_BLA(n)  for (auto& t : targets) f(t);
#define IMM_TARGETS_SLA(n)  for (auto& kv : targets) f(kv.second);
#define IMM_TARGETS_IVA(n)
#define IMM_TARGETS_I64A(n)
#define IMM_TARGETS_LA(n)
#define IMM_TARGETS_NLA(n)
#define IMM_TARGETS_ILA(n)
#define IMM_TARGETS_IA(n)
#define IMM_TARGETS_DA(n)
#define IMM_TARGETS_SA(n)
#define IMM_TARGETS_RATA(n)
#define IMM_TARGETS_AA(n)
#define IMM_TARGETS_BA(n)      f(target##n);
#define IMM_TARGETS_OA_IMPL(n)
#define IMM_TARGETS_OA(type)   IMM_TARGETS_OA_IMPL
#define IMM_TARGETS_VSA(n)
#define IMM_TARGETS_KA(n)
#define IMM_TARGETS_LAR(n)
#define IMM_TARGETS_ITA(n)
#define IMM_TARGETS_FCA(n)   if (fca.asyncEagerTarget() != NoBlockId) { \
                               f(fca.asyncEagerTarget());               \
                             }

#define IMM_MEM(which, n)          IMM_TY_##which IMM_NAME_##which(n)
#define IMM_MEM_NA
#define IMM_MEM_ONE(x)                IMM_MEM(x, 1);
#define IMM_MEM_TWO(x, y)             IMM_MEM(x, 1); IMM_MEM(y, 2);
#define IMM_MEM_THREE(x, y, z)        IMM_MEM(x, 1); IMM_MEM(y, 2); \
                                      IMM_MEM(z, 3);
#define IMM_MEM_FOUR(x, y, z, l)      IMM_MEM(x, 1); IMM_MEM(y, 2); \
                                      IMM_MEM(z, 3); IMM_MEM(l, 4);
#define IMM_MEM_FIVE(x, y, z, l, m)   IMM_MEM(x, 1); IMM_MEM(y, 2); \
                                      IMM_MEM(z, 3); IMM_MEM(l, 4); \
                                      IMM_MEM(m, 5);
#define IMM_MEM_SIX(x, y, z, l, m, n) IMM_MEM(x, 1); IMM_MEM(y, 2); \
                                      IMM_MEM(z, 3); IMM_MEM(l, 4); \
                                      IMM_MEM(m, 5); IMM_MEM(n, 6);

#define IMM_SERDE_SD(which, n)             sd(IMM_NAME_##which(n))
#define IMM_SERDE_SD_NA
#define IMM_SERDE_SD_ONE(x)                IMM_SERDE_SD(x, 1);
#define IMM_SERDE_SD_TWO(x, y)             IMM_SERDE_SD(x, 1); IMM_SERDE_SD(y, 2);
#define IMM_SERDE_SD_THREE(x, y, z)        IMM_SERDE_SD(x, 1); IMM_SERDE_SD(y, 2); \
                                           IMM_SERDE_SD(z, 3);
#define IMM_SERDE_SD_FOUR(x, y, z, l)      IMM_SERDE_SD(x, 1); IMM_SERDE_SD(y, 2); \
                                           IMM_SERDE_SD(z, 3); IMM_SERDE_SD(l, 4);
#define IMM_SERDE_SD_FIVE(x, y, z, l, m)   IMM_SERDE_SD(x, 1); IMM_SERDE_SD(y, 2); \
                                           IMM_SERDE_SD(z, 3); IMM_SERDE_SD(l, 4); \
                                           IMM_SERDE_SD(m, 5);
#define IMM_SERDE_SD_SIX(x, y, z, l, m, n) IMM_SERDE_SD(x, 1); IMM_SERDE_SD(y, 2); \
                                           IMM_SERDE_SD(z, 3); IMM_SERDE_SD(l, 4); \
                                           IMM_SERDE_SD(m, 5); IMM_SERDE_SD(n, 6);

#define IMM_SERDE_MAKE(which, n)             auto IMM_NAME_##which(n) = \
                                               sd.template make<IMM_TY_##which>()
#define IMM_SERDE_MAKE_NA
#define IMM_SERDE_MAKE_ONE(x)                IMM_SERDE_MAKE(x, 1);
#define IMM_SERDE_MAKE_TWO(x, y)             IMM_SERDE_MAKE(x, 1); IMM_SERDE_MAKE(y, 2);
#define IMM_SERDE_MAKE_THREE(x, y, z)        IMM_SERDE_MAKE(x, 1); IMM_SERDE_MAKE(y, 2); \
                                             IMM_SERDE_MAKE(z, 3);
#define IMM_SERDE_MAKE_FOUR(x, y, z, l)      IMM_SERDE_MAKE(x, 1); IMM_SERDE_MAKE(y, 2); \
                                             IMM_SERDE_MAKE(z, 3); IMM_SERDE_MAKE(l, 4);
#define IMM_SERDE_MAKE_FIVE(x, y, z, l, m)   IMM_SERDE_MAKE(x, 1); IMM_SERDE_MAKE(y, 2); \
                                             IMM_SERDE_MAKE(z, 3); IMM_SERDE_MAKE(l, 4); \
                                             IMM_SERDE_MAKE(m, 5);
#define IMM_SERDE_MAKE_SIX(x, y, z, l, m, n) IMM_SERDE_MAKE(x, 1); IMM_SERDE_MAKE(y, 2); \
                                             IMM_SERDE_MAKE(z, 3); IMM_SERDE_MAKE(l, 4); \
                                             IMM_SERDE_MAKE(m, 5); IMM_SERDE_MAKE(n, 6);

#define IMM_SERDE_ARG(which, n)             std::move(IMM_NAME_##which(n))
#define IMM_SERDE_ARG_NA
#define IMM_SERDE_ARG_ONE(x)                IMM_SERDE_ARG(x, 1)
#define IMM_SERDE_ARG_TWO(x, y)             IMM_SERDE_ARG(x, 1), IMM_SERDE_ARG(y, 2)
#define IMM_SERDE_ARG_THREE(x, y, z)        IMM_SERDE_ARG(x, 1), IMM_SERDE_ARG(y, 2), \
                                            IMM_SERDE_ARG(z, 3)
#define IMM_SERDE_ARG_FOUR(x, y, z, l)      IMM_SERDE_ARG(x, 1), IMM_SERDE_ARG(y, 2), \
                                            IMM_SERDE_ARG(z, 3), IMM_SERDE_ARG(l, 4)
#define IMM_SERDE_ARG_FIVE(x, y, z, l, m)   IMM_SERDE_ARG(x, 1), IMM_SERDE_ARG(y, 2), \
                                            IMM_SERDE_ARG(z, 3), IMM_SERDE_ARG(l, 4), \
                                            IMM_SERDE_ARG(m, 5)
#define IMM_SERDE_ARG_SIX(x, y, z, l, m, n) IMM_SERDE_ARG(x, 1), IMM_SERDE_ARG(y, 2), \
                                            IMM_SERDE_ARG(z, 3), IMM_SERDE_ARG(l, 4), \
                                            IMM_SERDE_ARG(m, 5), IMM_SERDE_ARG(n, 6)

#define IMM_EQ_WRAP(e, ...)       detail::eq_pairs(e, __VA_ARGS__)
#define IMM_EQ(which, n)          detail::eq_operand<     \
                                    IMM_TY_##which,       \
                                    imm::IMM_ID_##which   \
                                  > { \
                                    IMM_NAME_##which(n),  \
                                    o.IMM_NAME_##which(n) \
                                  }
#define IMM_EQ_NA                 detail::eq_operand<void*,imm::NA> { 0, 0 }
#define IMM_EQ_ONE(x)                IMM_EQ(x, 1)
#define IMM_EQ_TWO(x, y)             IMM_EQ_ONE(x), IMM_EQ(y, 2)
#define IMM_EQ_THREE(x, y, z)        IMM_EQ_TWO(x, y), IMM_EQ(z, 3)
#define IMM_EQ_FOUR(x, y, z, l)      IMM_EQ_THREE(x, y, z), IMM_EQ(l, 4)
#define IMM_EQ_FIVE(x, y, z, l, m)   IMM_EQ_FOUR(x, y, z, l), IMM_EQ(m, 5)
#define IMM_EQ_SIX(x, y, z, l, m, n) IMM_EQ_FIVE(x, y, z, l, m), IMM_EQ(n, 6)

#define IMM_HASH_WRAP(h, ...)       detail::hash_combine(h, __VA_ARGS__)
#define IMM_HASH(which, n)          detail::hash_operand<     \
                                      IMM_TY_##which,         \
                                      imm::IMM_ID_##which     \
                                    > { IMM_NAME_##which(n) }
#define IMM_HASH_NA                 detail::hash_operand<void*,imm::NA> { 0 }
#define IMM_HASH_ONE(x)                IMM_HASH(x, 1)
#define IMM_HASH_TWO(x, y)             IMM_HASH_ONE(x), IMM_HASH(y, 2)
#define IMM_HASH_THREE(x, y, z)        IMM_HASH_TWO(x, y), IMM_HASH(z, 3)
#define IMM_HASH_FOUR(x, y, z, l)      IMM_HASH_THREE(x, y, z), IMM_HASH(l, 4)
#define IMM_HASH_FIVE(x, y, z, l, m)   IMM_HASH_FOUR(x, y, z, l), IMM_HASH(m, 5)
#define IMM_HASH_SIX(x, y, z, l, m, n) IMM_HASH_FIVE(x, y, z, l, m), IMM_HASH(n, 6)

#define IMM_TARGETS_NA
#define IMM_TARGETS_ONE(x)           IMM_TARGETS_##x(1)
#define IMM_TARGETS_TWO(x,y)         IMM_TARGETS_ONE(x) IMM_TARGETS_##y(2)
#define IMM_TARGETS_THREE(x,y,z)     IMM_TARGETS_TWO(x,y) IMM_TARGETS_##z(3)
#define IMM_TARGETS_FOUR(x,y,z,l)    IMM_TARGETS_THREE(x,y,z) IMM_TARGETS_##l(4)
#define IMM_TARGETS_FIVE(x,y,z,l,m)  IMM_TARGETS_FOUR(x,y,z,l) IMM_TARGETS_##m(5)
#define IMM_TARGETS_SIX(x,y,z,l,m,n) IMM_TARGETS_FIVE(x,y,z,l,m) IMM_TARGETS_##n(6)

#define IMM_CTOR(which, n)         IMM_TY_##which IMM_NAME_##which(n)
#define IMM_CTOR_NA
#define IMM_CTOR_ONE(x)                IMM_CTOR(x, 1)
#define IMM_CTOR_TWO(x, y)             IMM_CTOR(x, 1), IMM_CTOR(y, 2)
#define IMM_CTOR_THREE(x, y, z)        IMM_CTOR(x, 1), IMM_CTOR(y, 2), \
                                       IMM_CTOR(z, 3)
#define IMM_CTOR_FOUR(x, y, z, l)      IMM_CTOR(x, 1), IMM_CTOR(y, 2), \
                                       IMM_CTOR(z, 3), IMM_CTOR(l, 4)
#define IMM_CTOR_FIVE(x, y, z, l, m)   IMM_CTOR(x, 1), IMM_CTOR(y, 2),     \
                                       IMM_CTOR(z, 3), IMM_CTOR(l, 4),     \
                                       IMM_CTOR(m, 5)
#define IMM_CTOR_SIX(x, y, z, l, m, n) IMM_CTOR(x, 1), IMM_CTOR(y, 2),     \
                                       IMM_CTOR(z, 3), IMM_CTOR(l, 4),     \
                                       IMM_CTOR(m, 5), IMM_CTOR(n, 6)

#define IMM_INIT(which, n)         IMM_NAME_##which(n) \
                                     ( std::move(IMM_NAME_##which(n)) )
#define IMM_INIT_NA
#define IMM_INIT_ONE(x)                : IMM_INIT(x, 1)
#define IMM_INIT_TWO(x, y)             : IMM_INIT(x, 1), IMM_INIT(y, 2)
#define IMM_INIT_THREE(x, y, z)        : IMM_INIT(x, 1), IMM_INIT(y, 2), \
                                         IMM_INIT(z, 3)
#define IMM_INIT_FOUR(x, y, z, l)      : IMM_INIT(x, 1), IMM_INIT(y, 2), \
                                         IMM_INIT(z, 3), IMM_INIT(l, 4)
#define IMM_INIT_FIVE(x, y, z, l, m)   : IMM_INIT(x, 1), IMM_INIT(y, 2),   \
                                         IMM_INIT(z, 3), IMM_INIT(l, 4),   \
                                         IMM_INIT(m, 5)
#define IMM_INIT_SIX(x, y, z, l, m, n) : IMM_INIT(x, 1), IMM_INIT(y, 2),   \
                                         IMM_INIT(z, 3), IMM_INIT(l, 4),   \
                                         IMM_INIT(m, 5), IMM_INIT(n, 6)

#define POP_UV  if (i == 0) return Flavor::U
#define POP_CV  if (i == 0) return Flavor::C
#define POP_CU  if (i == 0) return Flavor::CU
#define POP_CUV POP_CU

#define POP_NOV             uint32_t numPop() const { return 0; } \
                            Flavor popFlavor(uint32_t) const { not_reached(); }

#define POP_ONE(x)          uint32_t numPop() const { return 1; } \
                            Flavor popFlavor(uint32_t i) const {  \
                              POP_##x; not_reached();             \
                            }

#define POP_TWO(x, y)       uint32_t numPop() const { return 2; }   \
                            Flavor popFlavor(uint32_t i) const {    \
                              POP_##x; --i; POP_##y; not_reached(); \
                            }

#define POP_THREE(x, y, z)  uint32_t numPop() const { return 3; }   \
                            Flavor popFlavor(uint32_t i) const {    \
                              POP_##x; --i; POP_##y; --i; POP_##z;  \
                              not_reached();                        \
                            }

#define POP_MFINAL  uint32_t numPop() const { return arg1; } \
                    Flavor popFlavor(uint32_t) const { not_reached(); }

#define POP_C_MFINAL(n) uint32_t numPop() const { return arg1 + n; } \
                     Flavor popFlavor(uint32_t) const { not_reached(); }

#define POP_CMANY   uint32_t numPop() const { return arg1; }  \
                    Flavor popFlavor(uint32_t i) const {      \
                      assertx(i < numPop());                  \
                      return Flavor::C;                       \
                    }

#define POP_SMANY   uint32_t numPop() const { return keys.size(); }  \
                    Flavor popFlavor(uint32_t i) const {      \
                      assertx(i < numPop());                  \
                      return Flavor::C;                       \
                    }

#define POP_CUMANY  uint32_t numPop() const { return arg1; }  \
                    Flavor popFlavor(uint32_t i) const {      \
                      assertx(i < numPop());                  \
                      return Flavor::CU;                      \
                    }

#define POP_FCALL(nin, nobj)                                                   \
                    uint32_t numPop() const {                                  \
                      return fca.template numPop<nin>();                       \
                    }                                                          \
                    Flavor popFlavor(uint32_t i) const {                       \
                      return fca.template popFlavor<nin,nobj>(i);              \
                    }

#define PUSH_NOV          uint32_t numPush() const { return 0; }

#define PUSH_ONE(x)       uint32_t numPush() const { return 1; }

#define PUSH_TWO(x, y)    uint32_t numPush() const { return 2; }

#define PUSH_CMANY        uint32_t numPush() const { return arg1; }
#define PUSH_FCALL        uint32_t numPush() const { return fca.numRets(); }

#define O(opcode, imms, inputs, outputs, flags) \
  struct opcode {                               \
    static constexpr Op op = Op::opcode;        \
    explicit opcode (IMM_CTOR_##imms)           \
      IMM_INIT_##imms                           \
    {}                                          \
                                                \
    IMM_MEM_##imms                              \
    POP_##inputs                                \
    PUSH_##outputs                              \
                                                \
    bool operator==(const opcode& o) const {    \
      return equals(o, detail::eq_default{});   \
    }                                           \
                                                \
    bool operator!=(const opcode& o) const {    \
      return !(*this == o);                     \
    }                                           \
                                                \
    template<class E>                           \
    bool equals(const opcode& o, E e) const {   \
      return IMM_EQ_WRAP(e, IMM_EQ_##imms);     \
    }                                           \
                                                \
    size_t hash() const {                       \
      return IMM_HASH_WRAP(                     \
        detail::hasher_default{},               \
        IMM_HASH_##imms);                       \
    }                                           \
                                                \
    template<class H>                           \
    size_t hash(H h) const {                    \
      return IMM_HASH_WRAP(h, IMM_HASH_##imms); \
    }                                           \
                                                \
    template <typename F>                       \
    void forEachTarget(F&& f) {                 \
      IMM_TARGETS_##imms                        \
    }                                           \
    template <typename F>                       \
    void forEachTarget(F&& f) const {           \
      IMM_TARGETS_##imms                        \
    }                                           \
                                                \
    template <typename SerDe>                   \
    static opcode makeForSerde(SerDe& sd) {     \
      static_assert(SerDe::deserializing);      \
      IMM_SERDE_MAKE_##imms                     \
      return bc::opcode {                       \
        IMM_SERDE_ARG_##imms                    \
      };                                        \
    }                                           \
                                                \
    template <typename SerDe>                   \
    void serde(SerDe& sd) {                     \
      IMM_SERDE_SD_##imms                       \
    }                                           \
  };
OPCODES
#undef O

#undef PUSH_NOV
#undef PUSH_ONE
#undef PUSH_TWO
#undef PUSH_CMANY
#undef PUSH_FCALL

#undef POP_UV
#undef POP_CV
#undef POP_CU
#undef POP_CUV

#undef POP_NOV
#undef POP_ONE
#undef POP_TWO
#undef POP_THREE
#undef POP_MFINAL
#undef POP_C_MFINAL
#undef POP_CMANY
#undef POP_SMANY
#undef POP_CUMANY
#undef POP_FCALL

#undef IMM_TY_MA
#undef IMM_TY_BLA
#undef IMM_TY_SLA
#undef IMM_TY_IVA
#undef IMM_TY_I64A
#undef IMM_TY_LA
#undef IMM_TY_NLA
#undef IMM_TY_ILA
#undef IMM_TY_IA
#undef IMM_TY_DA
#undef IMM_TY_SA
#undef IMM_TY_RATA
#undef IMM_TY_AA
#undef IMM_TY_BA
#undef IMM_TY_OA
#undef IMM_TY_VSA
#undef IMM_TY_KA
#undef IMM_TY_LAR
#undef IMM_TY_ITA
#undef IMM_TY_FCA

// These are deliberately not undefined, so they can be used in other
// places.
// #undef IMM_NAME_BLA
// #undef IMM_NAME_SLA
// #undef IMM_NAME_IVA
// #undef IMM_NAME_I64A
// #undef IMM_NAME_LA
// #undef IMM_NAME_NLA
// #undef IMM_NAME_ILA
// #undef IMM_NAME_IA
// #undef IMM_NAME_DA
// #undef IMM_NAME_SA
// #undef IMM_NAME_RATA
// #undef IMM_NAME_AA
// #undef IMM_NAME_BA
// #undef IMM_NAME_OA
// #undef IMM_NAME_OA_IMPL
// #undef IMM_NAME_LAR
// #undef IMM_NAME_FCA

#undef IMM_TARGETS_BLA
#undef IMM_TARGETS_SLA
#undef IMM_TARGETS_IVA
#undef IMM_TARGETS_I64A
#undef IMM_TARGETS_LA
#undef IMM_TARGETS_NLA
#undef IMM_TARGETS_ILA
#undef IMM_TARGETS_IA
#undef IMM_TARGETS_DA
#undef IMM_TARGETS_SA
#undef IMM_TARGETS_RATA
#undef IMM_TARGETS_AA
#undef IMM_TARGETS_BA
#undef IMM_TARGETS_OA
#undef IMM_TARGETS_KA
#undef IMM_TARGETS_LAR
#undef IMM_TARGETS_ITA
#undef IMM_TARGETS_FCA

#undef IMM_TARGETS_NA
#undef IMM_TARGETS_ONE
#undef IMM_TARGETS_TWO
#undef IMM_TARGETS_THREE
#undef IMM_TARGETS_FOUR
#undef IMM_TARGETS_FIVE
#undef IMM_TARGETS_SIX

#undef IMM_MEM
#undef IMM_MEM_NA
#undef IMM_MEM_ONE
#undef IMM_MEM_TWO
#undef IMM_MEM_THREE
#undef IMM_MEM_FOUR
#undef IMM_MEM_FIVE
#undef IMM_MEM_SIX

#undef IMM_SERDE_SD
#undef IMM_SERDE_SD_NA
#undef IMM_SERDE_SD_ONE
#undef IMM_SERDE_SD_TWO
#undef IMM_SERDE_SD_THREE
#undef IMM_SERDE_SD_FOUR
#undef IMM_SERDE_SD_FIVE
#undef IMM_SERDE_SD_SIX

#undef IMM_SERDE_MAKE
#undef IMM_SERDE_MAKE_NA
#undef IMM_SERDE_MAKE_ONE
#undef IMM_SERDE_MAKE_TWO
#undef IMM_SERDE_MAKE_THREE
#undef IMM_SERDE_MAKE_FOUR
#undef IMM_SERDE_MAKE_FIVE
#undef IMM_SERDE_MAKE_SIX

#undef IMM_SERDE_ARG
#undef IMM_SERDE_ARG_NA
#undef IMM_SERDE_ARG_ONE
#undef IMM_SERDE_ARG_TWO
#undef IMM_SERDE_ARG_THREE
#undef IMM_SERDE_ARG_FOUR
#undef IMM_SERDE_ARG_FIVE
#undef IMM_SERDE_ARG_SIX

#undef IMM_EQ
#undef IMM_EQ_NA
#undef IMM_EQ_ONE
#undef IMM_EQ_TWO
#undef IMM_EQ_THREE
#undef IMM_EQ_FOUR
#undef IMM_EQ_FIVE
#undef IMM_EQ_SIX

#undef IMM_HASH
#undef IMM_HASH_DO
#undef IMM_HASH_NA
#undef IMM_HASH_ONE
#undef IMM_HASH_TWO
#undef IMM_HASH_THREE
#undef IMM_HASH_FOUR
#undef IMM_HASH_FIVE
#undef IMM_HASH_SIX

#undef IMM_CTOR
#undef IMM_CTOR_NA
#undef IMM_CTOR_ONE
#undef IMM_CTOR_TWO
#undef IMM_CTOR_THREE
#undef IMM_CTOR_FOUR
#undef IMM_CTOR_FIVE
#undef IMM_CTOR_SIX

#undef IMM_INIT
#undef IMM_INIT_NA
#undef IMM_INIT_ONE
#undef IMM_INIT_TWO
#undef IMM_INIT_THREE
#undef IMM_INIT_FOUR

}

//////////////////////////////////////////////////////////////////////

/*
 * Bytecode is a tagged-union that can hold any HHBC opcode struct
 * defined above.  You can visit a bytecode with a static_visitor
 * using visit(), defined below.
 *
 * Each Bytecode also carries a corresponding SrcLoc that indicates
 * which line of PHP code the bytecode was generated for.
 */
struct Bytecode {
  // Default construction creates a Nop.
  Bytecode()
    : op(Op::Nop)
    , Nop(bc::Nop{})
  {}

#define O(opcode, ...)                          \
  /* implicit */ Bytecode(bc::opcode data)      \
    : op(Op::opcode)                            \
  {                                             \
    new (&opcode) bc::opcode(std::move(data));  \
  }

  OPCODES

#undef O

  // Note: assuming bc::Nop is empty and has trivial dtor/ctor.

  Bytecode(const Bytecode& o) : op(Op::Nop) { *this = o; }
  Bytecode(Bytecode&& o) noexcept : op(Op::Nop) { *this = std::move(o); }

  Bytecode& operator=(const Bytecode& o) {
    destruct();
    op = Op::Nop;
    srcLoc = o.srcLoc;
#define O(opcode, ...) \
    case Op::opcode: new (&opcode) bc::opcode(o.opcode); break;
    switch (o.op) { OPCODES }
#undef O
    op = o.op;
    return *this;
  }

  Bytecode& operator=(Bytecode&& o) {
    destruct();
    srcLoc = o.srcLoc;
#define O(opcode, ...) \
    case Op::opcode: new (&opcode) bc::opcode(std::move(o.opcode)); break;
    switch (o.op) { OPCODES }
#undef O
    op = o.op;
    return *this;
  }

  ~Bytecode() { destruct(); }

  uint32_t numPop() const {
#define O(opcode, ...) \
    case Op::opcode: return opcode.numPop();
    switch (op) { OPCODES }
#undef O
    not_reached();
  }

  Flavor popFlavor(uint32_t i) const {
#define O(opcode, ...) \
    case Op::opcode: return opcode.popFlavor(i);
    switch (op) { OPCODES }
#undef O
    not_reached();
  }

  uint32_t numPush() const {
#define O(opcode, ...) \
    case Op::opcode: return opcode.numPush();
    switch (op) { OPCODES }
#undef O
    not_reached();
  }

  template <typename F>
  void forEachTarget(F&& f) const {
#define O(opcode, ...) \
    case Op::opcode: return opcode.forEachTarget(std::forward<F>(f));
    switch (op) { OPCODES }
#undef O
    not_reached();
  }

  template <typename F>
  void forEachTarget(F&& f) {
#define O(opcode, ...) \
    case Op::opcode: return opcode.forEachTarget(std::forward<F>(f));
    switch (op) { OPCODES }
#undef O
    not_reached();
  }

  template <typename SerDe> void serde(SerDe& sd) {
    if constexpr (SerDe::deserializing) {
      if (UNLIKELY(op != Op::Nop)) destruct();
      sd(op)(srcLoc);
#define O(opcode, ...)                                           \
      case Op::opcode:                                           \
      new (&opcode) bc::opcode{sd.template make<bc::opcode>()};  \
      break;
      switch (op) { OPCODES }
#undef O
      return;
    } else {
      sd(op)(srcLoc);
#define O(opcode, ...)                            \
      case Op::opcode: return opcode.serde(sd);
      switch (op) { OPCODES }
#undef O
      not_reached();
    }
  }

  Op op;
  int32_t srcLoc{-1};

#define O(opcode, ...) bc::opcode opcode;
  union { OPCODES };
#undef O

private:
  void destruct() {
    switch (op) {
#define O(opcode, ...)                          \
      case Op::opcode:                          \
        { typedef bc::opcode X;                 \
          this->opcode.~X(); }                  \
        break;
      OPCODES
#undef O
    }
  }
};

//////////////////////////////////////////////////////////////////////

inline bool operator==(const Bytecode& a, const Bytecode& b) {
  if (a.op != b.op) return false;
#define O(opcode, ...) case Op::opcode: return a.opcode == b.opcode;
  switch (a.op) { OPCODES }
#undef O
  not_reached();
}

inline bool operator!=(const Bytecode& a, const Bytecode& b) {
  return !(a == b);
}

template<class E>
inline bool equals(const Bytecode& a, const Bytecode& b, E equals) {
  if (a.op != b.op) return false;
#define O(opcode, ...)  \
  case Op::opcode: return a.opcode.equals(b.opcode, equals);
  switch (a.op) { OPCODES }
#undef O
  not_reached();
}

template<class H>
inline size_t hash(const Bytecode& b, H hasher) {
  auto hash = 14695981039346656037ULL;
  auto o = static_cast<size_t>(b.op);
  hash ^= o;
#define O(opcode, ...)  \
  case Op::opcode:      \
    return folly::hash::hash_combine(b.opcode.hash(hasher), hash);
  switch (b.op) { OPCODES }
#undef O
  not_reached();
}

inline size_t hash(const Bytecode& b) {
  return hash(b, bc::detail::hasher_default{});
}

//////////////////////////////////////////////////////////////////////

/*
 * Helper for making a Bytecode with a given srcLoc.
 *
 * Ex:
 *    auto b = bc_with_loc(something.srcLoc, bc::Nop {});
 */
template<class T> Bytecode bc_with_loc(int32_t loc, const T& t) {
  Bytecode b = t;
  b.srcLoc = loc;
  return b;
}

//////////////////////////////////////////////////////////////////////

/*
 * Visit a bytecode using a StaticVisitor, similar to
 * boost::apply_visitor or match().
 *
 * The `v' argument should be a function object that accepts a call
 * operator for all the bc::Foo types. Its result type should be
 * independent of bytecode, but may vary with constness.
 */
template<class Visit>
auto visit(Bytecode& b, Visit v) {
#define O(opcode, ...) case Op::opcode: return v(b.opcode);
  switch (b.op) { OPCODES }
#undef O
  not_reached();
}

template<class Visit>
auto visit(const Bytecode& b, Visit v) {
#define O(opcode, ...) case Op::opcode: return v(b.opcode);
  switch (b.op) { OPCODES }
#undef O
  not_reached();
}

//////////////////////////////////////////////////////////////////////

}

}
