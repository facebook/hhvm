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

#include "hphp/runtime/vm/class.h"

#include <folly/Optional.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
namespace jit {
struct SSATmp;
namespace irgen {
struct IRGS;
}
}
///////////////////////////////////////////////////////////////////////////////
struct RuntimeCoeffects {
  using storage_t = uint16_t;

  static RuntimeCoeffects fromValue(uint16_t value) {
    return RuntimeCoeffects{value};
  }

  static RuntimeCoeffects none() {
    return RuntimeCoeffects::fromValue(0);
  }

  static RuntimeCoeffects full() {
    return RuntimeCoeffects::fromValue(std::numeric_limits<storage_t>::max());
  }

  static RuntimeCoeffects defaults();
  static RuntimeCoeffects pure();
  static RuntimeCoeffects policied_of();
  static RuntimeCoeffects write_this_props();

  // This function is a placeholder to indicate that the correct coeffect needs
  // to be indentified and passed in its place
  static RuntimeCoeffects fixme() {
    return RuntimeCoeffects::defaults();
  }


  uint16_t value() const { return m_data; }

  const std::string toString() const;

  // Checks whether provided coeffects in `this` can call
  // required coeffects in `o`
  bool canCall(const RuntimeCoeffects o) const {
    // a & b == a
    // a & ~b == 0
    return (m_data & (~o.m_data)) == 0;
  }

  bool canCallWithWarning(const RuntimeCoeffects) const;

  // This operator is equivalent to | of [coeffectA | coeffectB]
  RuntimeCoeffects& operator&=(const RuntimeCoeffects);

private:
  explicit RuntimeCoeffects(uint16_t data) : m_data(data) {}
  storage_t m_data;
};

struct StaticCoeffects {
  using storage_t = RuntimeCoeffects::storage_t;

  const std::string toString() const;

  RuntimeCoeffects toAmbient() const;
  RuntimeCoeffects toRequired() const;

  // Returns the corresponding shallow bits for coeffects that are local
  RuntimeCoeffects toShallowWithLocals() const;

  static StaticCoeffects fromValue(uint16_t value) {
    return StaticCoeffects{value};
  }

  uint16_t value() const { return m_data; }

  static StaticCoeffects none() {
    return StaticCoeffects::fromValue(0);
  }

  static StaticCoeffects defaults();
  static StaticCoeffects write_this_props();

  // This operator is equivalent to | of [coeffectA | coeffectB]
  StaticCoeffects& operator&=(const StaticCoeffects);

  template<class SerDe>
  void serde(SerDe& sd) {
    sd(m_data);
  }

private:
  // Returns the local bits
  storage_t locals() const;

private:
  explicit StaticCoeffects(uint16_t data) : m_data(data) {}
  storage_t m_data;
};

static_assert(sizeof(StaticCoeffects) == sizeof(uint16_t), "");
static_assert(sizeof(StaticCoeffects) == sizeof(RuntimeCoeffects), "");

///////////////////////////////////////////////////////////////////////////////

struct CoeffectRule final {
  struct FunParam {};
  struct CCParam {};
  struct CCThis {};
  struct CCReified {};
  struct ClosureParentScope {};
  struct GeneratorThis {};
  struct Caller {};

  CoeffectRule() = default;

  /////////////////////////////////////////////////////////////////////////////
  // Native coeffect rules ////////////////////////////////////////////////////

  CoeffectRule(FunParam, uint32_t index)
    : m_type(Type::FunParam)
    , m_index(index)
  {}

  CoeffectRule(CCParam, uint32_t index, const StringData* ctx_name)
    : m_type(Type::CCParam)
    , m_index(index)
    , m_name(ctx_name)
  { assertx(ctx_name); }

  CoeffectRule(CCThis,
               std::vector<LowStringPtr> types,
               const StringData* ctx_name)
    : m_type(Type::CCThis)
    , m_types(types)
    , m_name(ctx_name)
  { assertx(ctx_name); }

  CoeffectRule(CCReified,
               bool is_class,
               uint32_t index,
               std::vector<LowStringPtr> types,
               const StringData* ctx_name)
    : m_type(Type::CCReified)
    , m_isClass(is_class)
    , m_index(index)
    , m_types(types)
    , m_name(ctx_name)
  { assertx(ctx_name); }

  explicit CoeffectRule(ClosureParentScope)
    : m_type(Type::ClosureParentScope)
  {}

  explicit CoeffectRule(GeneratorThis)
    : m_type(Type::GeneratorThis)
  {}

  explicit CoeffectRule(Caller)
    : m_type(Type::Caller)
  {}

  RuntimeCoeffects emit(const Func*, uint32_t, void*, RuntimeCoeffects) const;
  jit::SSATmp* emitJit(jit::irgen::IRGS&, const Func*,
                       uint32_t, jit::SSATmp*, jit::SSATmp*) const;

  bool isClosureParentScope() const;
  bool isGeneratorThis() const;
  bool isCaller() const;

  folly::Optional<std::string> toString(const Func*) const;
  std::string getDirectiveString() const;

  template<class SerDe>
  void serde(SerDe&);

private:
  enum class Type {
    Invalid = 0,

    FunParam,
    CCParam,
    CCThis,
    CCReified,
    ClosureParentScope,
    GeneratorThis,
    Caller,
  };

  Type m_type{Type::Invalid};
  bool m_isClass{false};
  uint32_t m_index{0};
  std::vector<LowStringPtr> m_types{};
  LowStringPtr m_name{nullptr};
};

///////////////////////////////////////////////////////////////////////////////
}
