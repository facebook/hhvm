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

enum CoeffectAttr : uint16_t {
  CEAttrNone = 0,
  // The RxLevel attrs are used to encode the maximum level of reactivity
  // of a function. RxNonConditional indicates level conditionality.
  CEAttrRxLevel0         = (1u << 0),
  CEAttrRxLevel1         = (1u << 1),
  CEAttrRxLevel2         = (1u << 2),
  CEAttrRxNonConditional = (1u << 3),
};

constexpr CoeffectAttr operator|(CoeffectAttr a, CoeffectAttr b) {
  return CoeffectAttr((uint16_t)a | (uint16_t)b);
}


inline CoeffectAttr& operator|=(CoeffectAttr& a, const CoeffectAttr& b) {
  return (a = CoeffectAttr((uint16_t)a | (uint16_t)b));
}

///////////////////////////////////////////////////////////////////////////////

enum class RxLevel : uint8_t {
  None               = 0,
  Local              = 1,
  Shallow            = 2,
  Rx                 = 3,
  Pure               = 4,
};

constexpr int kRxAttrShift = 0;
#define ASSERT_LEVEL(attr, rl) \
  static_assert(static_cast<RxLevel>(attr >> kRxAttrShift) == RxLevel::rl, "")
ASSERT_LEVEL(CEAttrRxLevel0, Local);
ASSERT_LEVEL(CEAttrRxLevel1, Shallow);
ASSERT_LEVEL((CEAttrRxLevel0 | CEAttrRxLevel1), Rx);
ASSERT_LEVEL(CEAttrRxLevel2, Pure);
#undef ASSERT_LEVEL

constexpr uint16_t kRxAttrMask =
  CEAttrRxLevel0 | CEAttrRxLevel1 | CEAttrRxLevel2;
constexpr uint16_t kRxLevelMask = 7u;
static_assert(kRxAttrMask >> kRxAttrShift == kRxLevelMask, "");
static_assert(CEAttrRxNonConditional == (8u << kRxAttrShift), "");


constexpr RxLevel rxLevelFromAttr(CoeffectAttr attrs) {
  return static_cast<RxLevel>(
    (static_cast<uint16_t>(attrs) >> kRxAttrShift) & kRxLevelMask
  );
}

constexpr bool rxConditionalFromAttr(CoeffectAttr attrs) {
  return !(attrs & CEAttrRxNonConditional);
}

constexpr CoeffectAttr rxMakeAttr(RxLevel level, bool conditional) {
  return static_cast<CoeffectAttr>(static_cast<uint16_t>(level) << kRxAttrShift)
    | (conditional ? CEAttrNone : CEAttrRxNonConditional);
}

CoeffectAttr rxAttrsFromAttrString(const std::string& a);
const char* rxAttrsToAttrString(CoeffectAttr a);

const char* rxLevelToString(RxLevel r);

constexpr bool funcAttrIsAnyRx(CoeffectAttr a) {
  return static_cast<uint16_t>(a) & kRxAttrMask;
}

constexpr bool funcAttrIsPure(CoeffectAttr a) {
  return static_cast<uint16_t>(a) & CEAttrRxLevel2;
}

bool rxEnforceCallsInLevel(RxLevel level);
RxLevel rxRequiredCalleeLevel(RxLevel level);

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

#define incl_HPHP_VM_RX_INL_H_
#include "hphp/runtime/vm/rx-inl.h"
#undef incl_HPHP_VM_RX_INL_H_

