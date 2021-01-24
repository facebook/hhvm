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

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
struct RuntimeCoeffects {
  enum Level : uint16_t {
    Default   = 0,
    RxShallow = 1,
    Rx        = 2,
    Pure      = 3,
  };

  explicit RuntimeCoeffects(Level level) : m_data(level) {}

  static RuntimeCoeffects none() {
    return RuntimeCoeffects{Level::Default};
  }

  static RuntimeCoeffects fromValue(uint16_t value) {
    return RuntimeCoeffects{static_cast<Level>(value)};
  }

  uint16_t value() const { return m_data; }

  const std::string toString() const;

  // Checks whether provided coeffects in `this` can call
  // required coeffects in `o`
  bool canCall(const RuntimeCoeffects& o) const {
    return m_data <= o.m_data;
  }

  bool canCallWithWarning(const RuntimeCoeffects& o) const {
    if (canCall(o)) return true;

    auto callerIsPure = m_data == Level::Pure;
    return (CoeffectsConfig::rxEnforcementLevel() < 2) &&
           (!callerIsPure || CoeffectsConfig::pureEnforcementLevel() < 2);
  }

private:
  Level m_data;
};

struct StaticCoeffects {
  enum class Level : uint16_t {
    None    = 0,
    Local   = 1,
    Shallow = 2,
    Rx      = 3,
    Pure    = 4,
  };

  bool isPure() const {
    return m_data == Level::Pure;
  }

  bool isAnyRx() const {
    return m_data == Level::Local ||
           m_data == Level::Shallow ||
           m_data == Level::Rx;
  }

  const char* toString() const;

  RuntimeCoeffects toAmbient() const;
  RuntimeCoeffects toRequired() const;

  static StaticCoeffects fromName(const std::string&);

  static StaticCoeffects none() { return StaticCoeffects{Level::None}; }
  static StaticCoeffects pure() { return StaticCoeffects{Level::Pure}; }

  // This operator is equivalent to & of [coeffectA & coeffectB]
  StaticCoeffects& operator|=(const StaticCoeffects& o) {
    m_data = std::max(m_data, o.m_data);
    return *this;
  }

  template<class SerDe>
  void serde(SerDe& sd) {
    sd(m_data);
  }

private:
  explicit StaticCoeffects(Level level) : m_data(level) {}
  Level m_data;
};

static_assert(sizeof(StaticCoeffects) == sizeof(uint16_t), "");
static_assert(sizeof(StaticCoeffects) == sizeof(RuntimeCoeffects), "");

///////////////////////////////////////////////////////////////////////////////

struct CoeffectRule final {
  struct CondRxArg {};
  struct CondRxImpl {};
  struct CondRxArgImpl {};

  struct FunParam {};
  struct CCParam {};
  struct CCThis {};

  CoeffectRule() = default;

  /////////////////////////////////////////////////////////////////////////////
  // Attribute based RX rules /////////////////////////////////////////////////

  CoeffectRule(CondRxArg, uint32_t index)
    : m_type(Type::ConditionalReactiveArg)
    , m_index(index)
  {}

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
      case Type::ConditionalReactiveArg:
        return folly::sformat(".rx_cond_rx_of_arg {};", m_index);
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
        case Type::ConditionalReactiveArg:
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
    ConditionalReactiveArg,
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
