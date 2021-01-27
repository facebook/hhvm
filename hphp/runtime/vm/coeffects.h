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
struct RuntimeCoeffects {
  using storage_t = uint16_t;

  static RuntimeCoeffects none() {
    return RuntimeCoeffects{0};
  }

  static RuntimeCoeffects fromValue(uint16_t value) {
    return RuntimeCoeffects{value};
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

private:
  explicit RuntimeCoeffects(uint16_t data) : m_data(data) {}
  storage_t m_data;
};

struct StaticCoeffects {
  using storage_t = RuntimeCoeffects::storage_t;

  const folly::Optional<std::string> toString() const;

  RuntimeCoeffects toAmbient() const;
  RuntimeCoeffects toRequired() const;

  static StaticCoeffects fromValue(uint16_t value) {
    return StaticCoeffects{value};
  }

  uint16_t value() const { return m_data; }

  static StaticCoeffects none() {
    return StaticCoeffects::fromValue(0);
  }

  // This operator is equivalent to & of [coeffectA & coeffectB]
  StaticCoeffects& operator|=(const StaticCoeffects);

  template<class SerDe>
  void serde(SerDe& sd) {
    sd(m_data);
  }

private:
  explicit StaticCoeffects(uint16_t data) : m_data(data) {}
  storage_t m_data;
};

static_assert(sizeof(StaticCoeffects) == sizeof(uint16_t), "");
static_assert(sizeof(StaticCoeffects) == sizeof(RuntimeCoeffects), "");

///////////////////////////////////////////////////////////////////////////////

struct CoeffectRule final {
  struct CondRxImpl {};
  struct CondRxArgImpl {};

  struct FunParam {};
  struct CCParam {};
  struct CCThis {};

  CoeffectRule() = default;

  /////////////////////////////////////////////////////////////////////////////
  // Attribute based RX rules /////////////////////////////////////////////////

  CoeffectRule(CondRxImpl, const StringData* name)
    : m_type(Type::ConditionalReactiveImplements)
    , m_name(name)
    , m_ne(NamedEntity::get(name))
  { assertx(name); }

  CoeffectRule(CondRxArgImpl, uint32_t index, const StringData* name)
    : m_type(Type::ConditionalReactiveArgImplements)
    , m_index(index)
    , m_name(name)
    , m_ne(NamedEntity::get(name))
  { assertx(name); }

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

  CoeffectRule(CCThis, const StringData* ctx_name)
    : m_type(Type::CCThis)
    , m_name(ctx_name)
  { assertx(ctx_name); }


  std::string getDirectiveString() const {
    switch (m_type) {
      case Type::ConditionalReactiveImplements:
        return folly::sformat(".rx_cond_implements \"{}\";",
                              folly::cEscape<std::string>(
                                m_name->toCppString()));
      case Type::ConditionalReactiveArgImplements:
        return folly::sformat(".rx_cond_implements_arg {} \"{}\";",
                              m_index,
                              folly::cEscape<std::string>(
                                m_name->toCppString()));
      case Type::FunParam:
        return folly::sformat(".coeffects_fun_param {};", m_index);
      case Type::CCParam:
        return folly::sformat(".coeffects_cc_param {} {};", m_index,
                              folly::cEscape<std::string>(
                                m_name->toCppString()));
      case Type::CCThis:
        return folly::sformat(".coeffects_cc_this {};",
                              folly::cEscape<std::string>(
                                m_name->toCppString()));
      case Type::Invalid:
        always_assert(false);
    }
    not_reached();
  }

  template<class SerDe>
  void serde(SerDe& sd) {
    sd(m_type)
      (m_index)
      (m_name)
    ;

    if (SerDe::deserializing) {
      switch (m_type) {
        case Type::ConditionalReactiveImplements:
        case Type::ConditionalReactiveArgImplements:
          m_ne = NamedEntity::get(m_name);
          break;
        case Type::FunParam:
        case Type::CCParam:
        case Type::CCThis:
          break;
        case Type::Invalid:
          always_assert(false);
      }
    }
  }

private:
  enum class Type {
    Invalid = 0,
    ConditionalReactiveImplements,
    ConditionalReactiveArgImplements,

    FunParam,
    CCParam,
    CCThis,
  };

  Type m_type{Type::Invalid};
  uint32_t m_index{0};
  LowPtr<const StringData> m_name{nullptr};
  LowPtr<const NamedEntity> m_ne{nullptr};
};

///////////////////////////////////////////////////////////////////////////////
}
