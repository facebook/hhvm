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

enum class RxLevel : uint8_t {
  None               = 0,
  Local              = 1,
  Shallow            = 2,
  Rx                 = 3,
  Pure               = 4,
};

constexpr int kRxAttrShift = 14;
#define ASSERT_LEVEL(attr, rl) \
  static_assert(static_cast<RxLevel>(attr >> kRxAttrShift) == RxLevel::rl, "")
ASSERT_LEVEL(AttrRxLevel0, Local);
ASSERT_LEVEL(AttrRxLevel1, Shallow);
ASSERT_LEVEL((AttrRxLevel0 | AttrRxLevel1), Rx);
ASSERT_LEVEL(AttrRxLevel2, Pure);
#undef ASSERT_LEVEL

constexpr uint32_t kRxAttrMask = AttrRxLevel0 | AttrRxLevel1 | AttrRxLevel2;
constexpr uint32_t kRxLevelMask = 7u;
static_assert(kRxAttrMask >> kRxAttrShift == kRxLevelMask, "");
static_assert(AttrRxNonConditional == (8u << kRxAttrShift), "");


constexpr RxLevel rxLevelFromAttr(Attr attrs) {
  return static_cast<RxLevel>(
    (static_cast<uint32_t>(attrs) >> kRxAttrShift) & kRxLevelMask
  );
}

constexpr bool rxConditionalFromAttr(Attr attrs) {
  return !(attrs & AttrRxNonConditional);
}

constexpr Attr rxMakeAttr(RxLevel level, bool conditional) {
  return static_cast<Attr>(static_cast<uint32_t>(level) << kRxAttrShift)
    | (conditional ? AttrNone : AttrRxNonConditional);
}

Attr rxAttrsFromAttrString(const std::string& a);
const char* rxAttrsToAttrString(Attr a);

const char* rxLevelToString(RxLevel r);

constexpr bool funcAttrIsAnyRx(Attr a) {
  return static_cast<uint32_t>(a) & kRxAttrMask;
}

constexpr bool funcAttrIsPure(Attr a) {
  return static_cast<uint32_t>(a) & AttrRxLevel2;
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

