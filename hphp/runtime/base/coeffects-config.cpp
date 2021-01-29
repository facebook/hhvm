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

#include "hphp/runtime/base/coeffects-config.h"

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
  X(rx)              \
  X(write_props)

#define CIPP_COEFFECTS \
  X(cipp_local)        \
  X(cipp_shallow)      \
  X(cipp)              \
  X(cipp_global)

#define COEFFECTS \
  RX_COEFFECTS    \
  CIPP_COEFFECTS

struct Coeffects {
  static constexpr auto s_defaults = "defaults";
  static constexpr auto s_pure = "pure";

#define X(x) static constexpr auto s_##x = #x;
  COEFFECTS
#undef X
};

struct Capabilities {
  static constexpr auto s_rx_defaults = "rx_defaults";
  static constexpr auto s_cipp_defaults = "cipp_defaults";

  static constexpr auto s_rx_pure = "rx_pure";
  static constexpr auto s_cipp_pure = "cipp_pure";

#define X(x) static constexpr auto s_##x = #x;
  COEFFECTS
#undef X
};

using C = Coeffects;
using Cap = Capabilities;

const hphp_fast_string_map<hphp_fast_set<std::string>> s_coeffects_to_capabilities{
  {C::s_defaults, {Cap::s_rx_defaults, Cap::s_cipp_defaults}},
  {C::s_pure, {Cap::s_rx_pure, Cap::s_cipp_pure}},

#define X(x) {C::s_##x, {Cap::s_##x, Cap::s_cipp_defaults}},
  RX_COEFFECTS
#undef X

#define X(x) {C::s_##x, {Cap::s_rx_defaults, Cap::s_##x}},
  CIPP_COEFFECTS
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
  FTRACE(5, "Adding edge {} -> {}\n", src->name, dst->name);
  src->children.push_back(dst);
  dst->parents.push_back(src);
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
  auto rx_pure = createNode(Cap::s_rx_pure);
  addEdges(createNode(Cap::s_rx_defaults, false, true),
           addEdges(createNode(Cap::s_rx, true),
                    rx_pure),
           addEdges(createNode(Cap::s_write_props),
                    rx_pure));

  addEdges(createNode(Cap::s_cipp_defaults, false, true),
           addEdges(createNode(Cap::s_cipp, true),
                    addEdges(createNode(Cap::s_cipp_global),
                             createNode(Cap::s_cipp_pure))));
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
  s_instance->m_cippLevel = 0;
  for (auto const [name, level] : map) {
    if (name == C::s_rx) s_instance->m_rxLevel = level;
    else if (name == C::s_cipp) s_instance->m_cippLevel = level;
    s_instance->m_pureLevel = std::max(s_instance->m_pureLevel, level);
  }
}

void CoeffectsConfig::initCapabilities() {
  storage_t escapeMask = 0;

  auto nextBit = 0;
  auto const add = [] (const std::string& name, storage_t value) {
    getCapabilityMap().insert({name, value});
    getCapabilityVector().push_back({name, value});
    FTRACE(1, "{:<14}: {:016b}\n", name, value);
  };

  for (auto cap_defaults : getCapabilityGraphs().entries) {
    add(cap_defaults->name, 0);

    std::queue<CapabilityNode*> queue;
    for (auto child : cap_defaults->children) queue.push(child);
    while (!queue.empty()) {
      auto cap = queue.front();
      queue.pop();

      // Am I already processed?
      if (getCapabilityMap().find(cap->name) != getCapabilityMap().end()) {
        continue;
      }

      storage_t bits = 0;
      bool again = false;
      // Are all my parents processed?
      for (auto parent : cap->parents) {
        auto const it = getCapabilityMap().find(parent->name);
        if (it != getCapabilityMap().end()) {
          bits |= it->second;
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
        // Set local
        add(folly::to<std::string>(cap->name, "_local"),
            bits | (1 << nextBit));
        escapeMask |= (1 << nextBit);

        // Set shallow
        add(folly::to<std::string>(cap->name, "_shallow"),
            bits | (1 << (nextBit + 1)));

        bits |= (1 << nextBit++);
      }
      bits |= (1 << nextBit++);
      add(cap->name, bits);
      for (auto child : cap->children) queue.push(child);
    }
  }

  assertx(s_instance);
  s_instance->m_numUsedBits = nextBit;
  s_instance->m_escapeMask = escapeMask;

  storage_t warningMask = 0;

  if (CoeffectsConfig::enabled()) {
    storage_t rxPure = getCapabilityMap().find(Cap::s_rx_pure)->second;
    storage_t cippPure = getCapabilityMap().find(Cap::s_cipp_pure)->second;
    if (CoeffectsConfig::pureEnforcementLevel() == 1) {
      warningMask = (rxPure | cippPure);
    } else {
      if (CoeffectsConfig::rxEnforcementLevel() == 1) warningMask |= rxPure;
      if (CoeffectsConfig::cippEnforcementLevel() == 1) warningMask |= cippPure;
    }
  }

  assertx(s_instance);
  s_instance->m_warningMask = warningMask;
}

std::string CoeffectsConfig::mangle() {
  assertx(s_instance);
  return folly::to<std::string>(
    C::s_pure, std::to_string(s_instance->m_pureLevel),
    C::s_rx, std::to_string(s_instance->m_rxLevel),
    C::s_cipp, std::to_string(s_instance->m_cippLevel)
  );
}


CoeffectsConfig::FromNameResult
CoeffectsConfig::fromName(const std::string& coeffect) {
  storage_t result = 0;
  bool isPure = false;
  bool isAnyRx = false;

  if (coeffect == C::s_pure) isPure = true;

#define X(x) if (coeffect == C::s_##x) isAnyRx = true;
  RX_COEFFECTS
#undef X

  auto const finish = [&] {
    return FromNameResult{StaticCoeffects::fromValue(result), isPure, isAnyRx};
  };

  if (!CoeffectsConfig::enabled()) return finish();

  if (!CoeffectsConfig::rxEnforcementLevel()) {
#define X(x) if (coeffect == C::s_##x) return finish();
  RX_COEFFECTS
#undef X
  }

  if (!CoeffectsConfig::cippEnforcementLevel()) {
#define X(x) if (coeffect == C::s_##x) return finish();
  CIPP_COEFFECTS
#undef X
  }

  auto const it = s_coeffects_to_capabilities.find(coeffect);
  if (it != s_coeffects_to_capabilities.end()) {
    for (auto const& capability: it->second) {
      auto const itt = getCapabilityMap().find(capability);
      always_assert(itt != getCapabilityMap().end());
      result |= itt->second;
    }
  }
  return finish();
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

std::vector<std::string>
CoeffectsConfig::toStringList(const StaticCoeffects data) {
  hphp_fast_set<std::string> capabilities;
  storage_t current = data.value();
  for (auto it = getCapabilityVector().rbegin();
       it != getCapabilityVector().rend();
       it++) {
    FTRACE(5, "Searching: current: {:016b}, looking at {}: {:016b}\n",
           current, it->first, it->second);
    if ((current & it->second) == it->second) {
      capabilities.insert(it->first);
      current &= (~it->second);
    }
  }

  always_assert(current == 0);

  FTRACE(3, "Converting {:016b} to string, found capabilities: '{}', ",
         data.value(), folly::join(", ", capabilities));

  std::vector<std::string> result;
  for (auto const& [name, caps] : s_coeffects_to_capabilities) {
    if (name == C::s_defaults) continue;
    if (std::all_of(caps.begin(),
                    caps.end(),
                    [&] (const std::string& s) {
                      return capabilities.count(s) > 0;
                    })) {
      result.push_back(name);
    }
  }

  FTRACE(3, "coeffects: {}\n", folly::join(" ", result));
  return result;
}

} // namespace HPHP
