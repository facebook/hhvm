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

#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/vm/class.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
enum RuntimeCoeffects : uint16_t {
  RCDefault   = 0,
  RCRxShallow = 1,
  RCRx        = 2,
  RCPure      = 3,
};

struct StaticCoeffects {
  enum class RxLevel : uint16_t {
    None    = 0,
    Local   = 1,
    Shallow = 2,
    Rx      = 3,
    Pure    = 4,
  };

  bool isPure() const {
    return m_data == RxLevel::Pure;
  }

  bool isAnyRx() const {
    return m_data == RxLevel::Local ||
           m_data == RxLevel::Shallow ||
           m_data == RxLevel::Rx;
  }

  const char* toString() const;
  const char* toUserDisplayString() const;

  RuntimeCoeffects toAmbient() const;
  RuntimeCoeffects toRequired() const;

  static StaticCoeffects fromName(const std::string&);

  static StaticCoeffects none() { return {RxLevel::None}; }
  static StaticCoeffects pure() { return {RxLevel::Pure}; }

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
  StaticCoeffects(RxLevel level) : m_data(level) {}
  RxLevel m_data;
};

static_assert(sizeof(StaticCoeffects) == sizeof(uint16_t), "");
static_assert(sizeof(StaticCoeffects) == sizeof(RuntimeCoeffects), "");

///////////////////////////////////////////////////////////////////////////////

struct CoeffectRule final {
  struct CondRxArg {};
  struct CondRxImpl {};
  struct CondRxArgImpl {};

  CoeffectRule() = default;

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
    ConditionalReactiveArgImplements
  };

  Type m_type{Type::Invalid};
  uint32_t m_index{0};
  LowPtr<const StringData> m_name{nullptr};
  LowPtr<const NamedEntity> m_ne{nullptr};
};

///////////////////////////////////////////////////////////////////////////////
}
