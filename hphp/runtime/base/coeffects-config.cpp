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

#include "hphp/hack/src/hackc/hhbc-unit.h"
#include "hphp/runtime/base/coeffects-config.h"
#include "hphp/runtime/vm/coeffects.h"

#include "hphp/util/hash-set.h"
#include "hphp/util/trace.h"

#include <queue>

TRACE_SET_MOD(coeffects);

namespace HPHP {

std::unique_ptr<CoeffectsConfig> CoeffectsConfig::s_instance;

namespace {
using storage_t = StaticCoeffects::storage_t;

static hphp_fast_string_map<storage_t>& getCapabilityMap() {
  static hphp_fast_string_map<storage_t> capabilityMap = {};
  return capabilityMap;
}

static std::vector<std::pair<std::string, storage_t>>& getCapabilityVector() {
  static std::vector<std::pair<std::string, storage_t>> capabilityVector = {};
  return capabilityVector;
}

struct CapabilityCombinator {
  size_t pos;
  size_t size;
};

static std::vector<CapabilityCombinator>& getCapabilityCombinator() {
  static std::vector<CapabilityCombinator> combinator = {};
  return combinator;
}

#define RX_COEFFECTS \
  X(rx_local)        \
  X(rx_shallow)      \
  X(rx)

#define ZONED_COEFFECTS \
  X(zoned_with)         \
  X(zoned_local)        \
  X(zoned_shallow)      \
  X(zoned)              \
  X(leak_safe_local)    \
  X(leak_safe_shallow)  \
  X(leak_safe)          \
  X(globals)            \
  X(read_globals)       \
  X(write_this_props)   \
  X(write_props)

#define COEFFECTS    \
  RX_COEFFECTS       \
  ZONED_COEFFECTS

struct Coeffects {
  static constexpr auto s_defaults = "defaults";
  static constexpr auto s_pure = "pure";

  static constexpr auto s_86backdoor = "86backdoor";
  static constexpr auto s_86backdoor_globals_leak_safe =
    "86backdoor_globals_leak_safe";

#define X(x) static constexpr auto s_##x = #x;
  COEFFECTS
#undef X
};

struct Capabilities {
  static constexpr auto s_rx_defaults = "rx_defaults";
  static constexpr auto s_zoned_defaults = "zoned_defaults";
  static constexpr auto s_zoned_unreachable = "zoned_unreachable";

  static constexpr auto s_rx_pure = "rx_pure";
  static constexpr auto s_zoned_maybe = "zoned_maybe";

#define X(x) static constexpr auto s_##x = #x;
  COEFFECTS
#undef X
};

using C = Coeffects;
using Cap = Capabilities;

const hphp_fast_string_map<hphp_fast_set<std::string>> s_coeffects_to_capabilities{
  {C::s_defaults, {Cap::s_rx_defaults, Cap::s_zoned_defaults}},
  {C::s_pure, {Cap::s_rx_pure, Cap::s_zoned_maybe}},

#define X(x) {C::s_##x, {Cap::s_##x, Cap::s_zoned_defaults}},
  RX_COEFFECTS
#undef X

#define X(x) {C::s_##x, {Cap::s_rx_defaults, Cap::s_##x}},
  ZONED_COEFFECTS
#undef X

};

struct CapabilityNode {
  CapabilityNode(const std::string& name, bool escape)
  : name(name)
  , escape(escape)
  , children({})
  , parents({})
  {}

  std::string name;
  bool escape;
  std::vector<CapabilityNode*> children;
  std::vector<CapabilityNode*> parents;
};

struct CapabilityGraphs {
  hphp_fast_string_map<std::unique_ptr<CapabilityNode>> map;
  std::vector<CapabilityNode*> entries;
};

static CapabilityGraphs& getCapabilityGraphs() {
  static CapabilityGraphs capabilityGraphs = {};
  return capabilityGraphs;
}

template <typename T>
T addEdges(T src, T dst) {
  FTRACE(5, "Adding edge {} -> {}\n", dst->name, src->name);
  dst->children.push_back(src);
  src->parents.push_back(dst);
  return src;
}

template<typename T, typename... Args>
T addEdges(T src, T dst, Args... args) {
  addEdges(src, dst);
  return addEdges(src, args...);
}

CapabilityNode* createNode(const std::string& name,
                           bool escape = false,
                           bool entry = false) {
  auto node = std::make_unique<CapabilityNode>(name, escape);
  auto ptr = node.get();
  if (entry) getCapabilityGraphs().entries.push_back(ptr);
  getCapabilityGraphs().map.insert({name, std::move(node)});
  return ptr;
}

void initCapabilityGraphs() {
  auto rx_pure = createNode(Cap::s_rx_pure, false, true);
  addEdges(createNode(Cap::s_rx_defaults),
           addEdges(createNode(Cap::s_rx, true),
                    rx_pure));

  auto zoned = createNode(Cap::s_zoned, true);
  auto read_globals = createNode(Cap::s_read_globals);
  auto zoned_maybe = createNode(Cap::s_zoned_maybe, false, true);
  addEdges(createNode(Cap::s_zoned_unreachable),
           addEdges(createNode(Cap::s_zoned_defaults),
                    addEdges(createNode(Cap::s_globals),
                             read_globals),
                    zoned),
           addEdges(createNode(Cap::s_zoned_with, true),
                    addEdges(zoned,
                             addEdges(createNode(Cap::s_leak_safe, true),
                                      addEdges(createNode(Cap::s_write_props),
                                               addEdges(createNode(Cap::s_write_this_props),
                                                        zoned_maybe)),
                                      addEdges(read_globals,
                                               zoned_maybe)))));
}

} //namespace

void CoeffectsConfig::init(const std::unordered_map<std::string, int>& map) {
  initEnforcementLevel(map);
  initCapabilityGraphs();
  initCapabilities();
}

void CoeffectsConfig::initEnforcementLevel(
  const std::unordered_map<std::string, int>& map
) {
  assertx(!s_instance);
  s_instance = std::make_unique<CoeffectsConfig>();
  // Purity enforcement must be at least as strong as the highest level of
  // enforcement otherwise the whole coeffect system breaks
  s_instance->m_pureLevel = 0;
  s_instance->m_rxLevel = 0;
  s_instance->m_zonedLevel = 0;
  for (auto const& [name, level] : map) {
    if (name == C::s_rx) s_instance->m_rxLevel = level;
    else if (name == C::s_zoned || name == "policied") s_instance->m_zonedLevel = level;
    s_instance->m_pureLevel = std::max(s_instance->m_pureLevel, level);
  }
}

void CoeffectsConfig::initCapabilities() {
  storage_t escapeMask = 0;

  auto nextBit = 0;
  auto const add = [] (const std::string& name, storage_t value) {
    getCapabilityMap().insert({name, value});
    getCapabilityVector().push_back({name, value});
    FTRACE(1, "{:<21}: {:016b}\n", name, value);
  };

  for (auto cap_parents : getCapabilityGraphs().entries) {
    add(cap_parents->name, 0);

    std::queue<CapabilityNode*> queue;
    for (auto child : cap_parents->children) queue.push(child);
    while (!queue.empty()) {
      auto cap = queue.front();
      queue.pop();

      // Am I already processed?
      if (getCapabilityMap().find(cap->name) != getCapabilityMap().end()) {
        continue;
      }

      storage_t bits = 0;
      storage_t bits_shallow = 0;
      storage_t bits_local = 0;
      bool again = false;
      // Are all my parents processed?
      for (auto parent : cap->parents) {
        auto const it = getCapabilityMap().find(parent->name);
        if (it != getCapabilityMap().end()) {
          if (parent->escape) {
            auto const it_shallow =
              getCapabilityMap().find(folly::to<std::string>(parent->name,
                                                             "_shallow"));
            assertx(it_shallow != getCapabilityMap().end());
            bits_shallow |= it_shallow->second;
            auto const it_local =
              getCapabilityMap().find(folly::to<std::string>(parent->name,
                                                             "_local"));
            assertx(it_local != getCapabilityMap().end());
            bits_local |= it_local->second;
            bits |= (cap->escape ? it->second : bits_shallow);
          } else {
            bits_shallow |= it->second;
            bits_local |= it->second;
            bits |= it->second;
          }
        } else {
          again = true;
          break;
        }
      }
      if (again) continue;

      always_assert(nextBit < std::numeric_limits<storage_t>::digits);

      getCapabilityCombinator().push_back(
        {static_cast<size_t>(nextBit),
         static_cast<size_t>(cap->escape ? 2 : 1)});

      if (cap->escape) {
        always_assert(nextBit + 1 < std::numeric_limits<storage_t>::digits);
        // Set local (01)
        add(folly::to<std::string>(cap->name, "_local"),
            bits_local | (1 << nextBit));
        escapeMask |= (1 << nextBit);

        // Set full (10)
        add(cap->name, bits | (1 << (nextBit + 1)));

        // Set shallow (11)
        add(folly::to<std::string>(cap->name, "_shallow"),
            bits_shallow | (3 << nextBit));

        nextBit += 2;
      } else {
        add(cap->name, bits | (1 << nextBit++));
      }
      for (auto child : cap->children) queue.push(child);
    }
  }

  assertx(s_instance);
  s_instance->m_numUsedBits = nextBit;
  s_instance->m_escapeMask = escapeMask;

  storage_t warningMask = 0;

  if (CoeffectsConfig::enabled()) {
    storage_t rxMask = getCapabilityMap().find(Cap::s_rx_defaults)->second;
    storage_t zonedMask =
      getCapabilityMap().find(Cap::s_zoned_unreachable)->second;
    if (CoeffectsConfig::pureEnforcementLevel() == 1) {
      warningMask = (rxMask | zonedMask);
    } else if (CoeffectsConfig::rxEnforcementLevel() == 1) {
        warningMask |= rxMask;
    } else if (CoeffectsConfig::zonedEnforcementLevel() == 1) {
        warningMask |= zonedMask;
    }
  }

  assertx(s_instance);
  s_instance->m_warningMask = warningMask;
}

void CoeffectsConfig::importForParse(const CoeffectsConfig& c) {
  s_instance = std::make_unique<CoeffectsConfig>(c);
}

std::string CoeffectsConfig::mangle() {
  assertx(s_instance);
  return folly::to<std::string>(
    C::s_pure, std::to_string(s_instance->m_pureLevel),
    C::s_rx, std::to_string(s_instance->m_rxLevel),
    C::s_zoned, std::to_string(s_instance->m_zonedLevel)
  );
}

bool CoeffectsConfig::isPure(const StringData* name) {
  return name->toCppString() == C::s_pure;
}

bool CoeffectsConfig::isAnyRx(const StringData* sd) {
  auto const name = sd->toCppString();
  return name == C::s_rx ||
         name == C::s_rx_shallow ||
         name == C::s_rx_local;
}

StaticCoeffects CoeffectsConfig::fromName(const std::string& coeffect) {
  if (!CoeffectsConfig::enabled()) return StaticCoeffects::none();

  if (!CoeffectsConfig::rxEnforcementLevel()) {
#define X(x) if (coeffect == C::s_##x) return StaticCoeffects::defaults();
  RX_COEFFECTS
#undef X
  }

  if (!CoeffectsConfig::zonedEnforcementLevel()) {
#define X(x) if (coeffect == C::s_##x) return StaticCoeffects::defaults();
  ZONED_COEFFECTS
#undef X
  }

  if (coeffect == C::s_86backdoor ||
      coeffect == C::s_86backdoor_globals_leak_safe) {
    return CoeffectsConfig::fromName(C::s_pure);
  }

  auto const it = s_coeffects_to_capabilities.find(coeffect);
  if (it == s_coeffects_to_capabilities.end() || it->second.empty()) {
    return StaticCoeffects::defaults();
  }

  storage_t result = 0;
  for (auto const& capability: it->second) {
    auto const itt = getCapabilityMap().find(capability);
    always_assert(itt != getCapabilityMap().end());
    result |= itt->second;
  }
  return StaticCoeffects::fromValue(result);
}

RuntimeCoeffects CoeffectsConfig::escapesTo(const std::string& coeffect) {
  if (CoeffectsConfig::rxEnforcementLevel()) {
    if (coeffect == C::s_rx_local) return RuntimeCoeffects::defaults();
  }
  if (CoeffectsConfig::zonedEnforcementLevel()) {
    if (coeffect == C::s_zoned_local) return RuntimeCoeffects::defaults();
    if (coeffect == C::s_leak_safe_local) return RuntimeCoeffects::defaults();
  }
  if (coeffect == C::s_86backdoor) return RuntimeCoeffects::defaults();
  if (coeffect == C::s_86backdoor_globals_leak_safe) {
    return RuntimeCoeffects::globals_leak_safe();
  }
  return RuntimeCoeffects::none();
}

StaticCoeffects CoeffectsConfig::combine(const StaticCoeffects a,
                                         const StaticCoeffects b) {
  storage_t result = 0;
  for (auto entry : getCapabilityCombinator()) {
    auto const mask = ((1 << entry.size) - 1) << entry.pos;
    result |= std::max(a.value() & mask, b.value() & mask);
  }
  return StaticCoeffects::fromValue(result);
}

std::string CoeffectsConfig::fromHackCCtx(const hackc::hhbc::Ctx& ctx) {
  switch (ctx) {
    case hackc::hhbc::Ctx::Defaults:       return C::s_defaults;
    case hackc::hhbc::Ctx::WriteThisProps: return C::s_write_this_props;
    case hackc::hhbc::Ctx::WriteProps:     return C::s_write_props;
    case hackc::hhbc::Ctx::RxLocal:        return C::s_rx_local;
    case hackc::hhbc::Ctx::RxShallow:      return C::s_rx_shallow;
    case hackc::hhbc::Ctx::Rx:             return C::s_rx;
    case hackc::hhbc::Ctx::ZonedWith:      return C::s_zoned_with;
    case hackc::hhbc::Ctx::ZonedLocal:     return C::s_zoned_local;
    case hackc::hhbc::Ctx::ZonedShallow:   return C::s_zoned_shallow;
    case hackc::hhbc::Ctx::Zoned:          return C::s_zoned;
    case hackc::hhbc::Ctx::LeakSafeLocal:  return C::s_leak_safe_local;
    case hackc::hhbc::Ctx::LeakSafeShallow:return C::s_leak_safe_shallow;
    case hackc::hhbc::Ctx::LeakSafe:       return C::s_leak_safe;
    case hackc::hhbc::Ctx::ReadGlobals:    return C::s_read_globals;
    case hackc::hhbc::Ctx::Globals:        return C::s_globals;
    case hackc::hhbc::Ctx::Pure:           return C::s_pure;
  }
  not_reached();
}

std::vector<std::string>
CoeffectsConfig::toStringList(const StaticCoeffects input) {
  hphp_fast_set<std::string> capabilities;
  storage_t current = 0;
  for (auto it = getCapabilityVector().rbegin();
       it != getCapabilityVector().rend();
       it++) {
    // if capability is entirely covered by the input, and
    // it adds a new bit to current
    if ((input.value() & it->second) == it->second &&
        (current | it->second) != current) {
      capabilities.insert(it->first);
      current |= it->second;
    }
    if (current == input.value()) break;
  }

  assert_flog(current == input.value(),
              "Trying to find a context for {:016b}, only found {:016b}",
              input.value(), current);

  std::vector<std::string> result;
  for (auto const& [name, caps] : s_coeffects_to_capabilities) {
    if (std::all_of(caps.begin(),
                    caps.end(),
                    [&] (const std::string& s) {
                      return capabilities.count(s) > 0;
                    })) {
      result.push_back(name);
    }
  }
  // Alphabetically sort them so that the contents of the error messages
  // always appear in the same order
  std::sort(result.begin(), result.end());
  return result;
}

} // namespace HPHP
