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

#include "hphp/runtime/vm/coeffects.h"

#include "hphp/runtime/base/coeffects-config.h"
#include "hphp/runtime/vm/blob-helper.h"
#include "hphp/util/trace.h"

TRACE_SET_MOD(coeffects);

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const std::string RuntimeCoeffects::toString() const {
  // Pretend to be StaticCoeffects, this is safe since RuntimeCoeffects is a
  // subset of StaticCoeffects
  auto const data = StaticCoeffects::fromValue(m_data);
  auto const list = CoeffectsConfig::toStringList(data);
  if (list.empty()) return "defaults";
  if (list.size() == 1 && list[0] == "pure") return "";
  return folly::join(", ", list);
}

bool RuntimeCoeffects::canCallWithWarning(const RuntimeCoeffects o) const {
  auto const promoted =
    RuntimeCoeffects::fromValue(o.m_data | CoeffectsConfig::warningMask());
  return canCall(promoted);
}

const folly::Optional<std::string> StaticCoeffects::toString() const {
  auto const list = CoeffectsConfig::toStringList(*this);
  if (list.empty()) return folly::none;
  return folly::join(" ", list);
}

RuntimeCoeffects StaticCoeffects::toAmbient() const {
  auto const locals =
    (((~m_data) >> 1) & m_data) & CoeffectsConfig::escapeMask();
  auto const val = m_data - locals;
  FTRACE(5, "Converting {:016b} to ambient {:016b}\n", m_data, val);
  return RuntimeCoeffects::fromValue(val);
}

RuntimeCoeffects StaticCoeffects::toRequired() const {
  auto const locals =
    (((~m_data) >> 1) & m_data) & CoeffectsConfig::escapeMask();
  // This converts the 01 (local) pattern to 10 (shallow) pattern
  // (m_data | (locals << 1)) & (~locals)
  // => m_data - locals + 2 * locals
  // => m_data + locals
  auto const val = m_data + locals;
  FTRACE(5, "Converting {:016b} to required {:016b}\n", m_data, val);
  return RuntimeCoeffects::fromValue(val);
}

RuntimeCoeffects& RuntimeCoeffects::operator&=(const RuntimeCoeffects o) {
  m_data &= o.m_data;
  return *this;
}

StaticCoeffects& StaticCoeffects::operator|=(const StaticCoeffects o) {
  return (*this = CoeffectsConfig::combine(*this, o));
}

folly::Optional<std::string> CoeffectRule::toString(const Func* f) const {
  switch (m_type) {
    case Type::ConditionalReactiveImplements:
    case Type::ConditionalReactiveArgImplements:
      return folly::none;
    case Type::FunParam:
      return folly::to<std::string>("ctx $",
                                    f->localVarName(m_index)->toCppString());
    case Type::CCParam:
      return folly::to<std::string>("$",
                                    f->localVarName(m_index)->toCppString(),
                                    "::",
                                    m_name->toCppString());
    case Type::CCThis:
      return folly::to<std::string>("this::", m_name->toCppString());
    case Type::Invalid:
      always_assert(false);
  }
  not_reached();
}

folly::Optional<RuntimeCoeffects>
CoeffectRule::emit(const Func* f, uint32_t numArgsInclUnpack) const {
  switch (m_type) {
    case Type::CCParam: {
      if (m_index >= numArgsInclUnpack) return folly::none;
      auto const index =
        numArgsInclUnpack - 1 - m_index + (f->hasReifiedGenerics() ? 1 : 0);
      auto const tv = vmStack().indC(index);
      if (tvIsNull(tv)) return folly::none;
      if (!tvIsObject(tv)) {
        raise_error(folly::sformat("Coeffect rule requires parameter at "
                                   "position {} to be an object or null",
                                   m_index));
      }
      auto const cls = tv->m_data.pobj->getVMClass();
      return cls->clsCtxCnsGet(m_name);
    }
    case Type::ConditionalReactiveImplements:
    case Type::ConditionalReactiveArgImplements:
    case Type::FunParam:
    case Type::CCThis:
      return folly::none;
    case Type::Invalid:
      always_assert(false);
  }
  not_reached();
}

std::string CoeffectRule::getDirectiveString() const {
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
void CoeffectRule::serde(SerDe& sd) {
  sd(m_type)
    (m_index)
    (m_name)
  ;

  if constexpr (SerDe::deserializing) {
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

template void CoeffectRule::serde<>(BlobDecoder&);
template void CoeffectRule::serde<>(BlobEncoder&);

///////////////////////////////////////////////////////////////////////////////
}
