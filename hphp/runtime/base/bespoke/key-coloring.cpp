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

#include "hphp/runtime/base/bespoke/key-coloring.h"

#include "hphp/runtime/base/bespoke/struct-dict.h"

namespace HPHP { namespace bespoke {

TRACE_SET_MOD(bespoke);

//////////////////////////////////////////////////////////////////////////////

namespace {

struct ColoringMetadata {
  Optional<ColorMap> coloring;
  LayoutWeightVector coloringAcceptedLayouts;
  LayoutWeightVector coloringDiscardedLayouts;
};

ColoringMetadata s_metadata;

//////////////////////////////////////////////////////////////////////////////

Optional<ColorMap> performColoring(
    const LayoutWeightVector::const_iterator& begin,
    const LayoutWeightVector::const_iterator& end) {
  FTRACE(3, "Attempting to color {} layouts.\n", std::distance(begin, end));

  if (begin == end) return {ColorMap()};

  // 1. Construct the interference graph from the layout set.
  using VertexSet = folly::F14FastSet<const StringData*>;
  using EdgeSet = folly::F14FastMap<const StringData*, VertexSet>;
  auto allEdges = [&] {
    auto result = EdgeSet();
    std::for_each(begin, end, [&](auto const& pair) {
      auto const& layout = pair.first;
      auto const& keyOrder = layout->keyOrder();
      for (auto const& first : keyOrder) {
        result.try_emplace(first);
        for (auto const& second : keyOrder) {
          if (first == second) continue;
          result[first].insert(second);
          result[second].insert(first);
        }
      }
    });
    return result;
  }();

  // 2. Sort vertices in order of descending degree.
  using VertexData = std::pair<const StringData*, VertexSet>;
  using OrderedVertices = std::vector<VertexData>;
  auto const orderedVertices = [&] {
    auto result = OrderedVertices();
    result.reserve(allEdges.size());
    allEdges.eraseInto(
      allEdges.begin(), allEdges.end(),
      [&](auto&& key, auto&& val) {
        result.emplace_back(std::move(key), std::move(val));
      }
    );
    std::sort(
      result.begin(),
      result.end(),
      [](auto const& a, auto const& b) {
        return a.second.size() > b.second.size();
      }
    );
    return result;
  }();

  // 3. Greedily color each vertex, assigning the lowest color available to not
  // interfere with adjacent vertices.
  using ColorSet = folly::F14FastSet<Color>;
  auto colors = ColorMap();
  for (auto const& pair : orderedVertices) {
    auto const& vertex = pair.first;
    auto const& edges = pair.second;
    auto colorsUsed = ColorSet();
    for (auto const& dest : edges) {
      auto const it = colors.find(dest);
      if (it == colors.end()) continue;
      colorsUsed.insert(it->second);
    }
    auto const colorAssigned = [&] {
      for (Color i = 1; i <= StructLayout::kMaxColor; i++) {
        if (colorsUsed.find(i) == colorsUsed.end()) {
          colors[vertex] = i;
          return true;
        }
      }
      return false;
    }();
    if (!colorAssigned) {
      FTRACE(3, "Colors exhausted while coloring {} layouts.\n",
             std::distance(begin, end));
      return std::nullopt;
    }
  }

  // 4. Validate the coloring.
  if constexpr (debug) {
    std::for_each(begin, end, [&](auto const& pair) {
      auto const& layout = pair.first;
      auto const& keyOrder = layout->keyOrder();
      for (auto const& first : keyOrder) {
        assertx(colors[first] > 0 && colors[first] <= StructLayout::kMaxColor);
        for (auto const& second : keyOrder) {
          if (first == second) continue;
          assertx(colors[first] != colors[second]);
        }
      }
    });
  }

  if (Trace::moduleEnabled(Trace::bespoke, 3)) {
    auto const DEBUG_ONLY maxColor = std::max_element(
      colors.cbegin(),
      colors.cend(),
      [](auto const& a, auto const& b) {
        return a.second < b.second;
      }
    );
    assertx(maxColor != colors.cend());
    FTRACE(3, "Colored {} layouts with {} colors.\n", std::distance(begin, end),
           maxColor->second);
  }

  return {colors};
}
}

//////////////////////////////////////////////////////////////////////////////

std::pair<LayoutWeightVector::const_iterator, Optional<ColorMap>>
 findKeyColoring(LayoutWeightVector& layouts) {
  FTRACE(3, "Attempting to color a prefix of {} layouts.\n", layouts.size());
  if (layouts.empty()) return {layouts.begin(), {}};

  // Sort the layouts in descending order by weight.
  std::sort(
    layouts.begin(), layouts.end(),
    [](auto const& a, auto const& b) { return a.second > b.second; }
  );

  // Binary search for a colorable prefix.
  auto lo_idx = 0;
  auto hi_idx = layouts.size();
  while (lo_idx + 1 != hi_idx) {
    auto const mid_idx = (lo_idx + hi_idx) / 2;
    auto const result = performColoring(
      layouts.begin(),
      layouts.begin() + (mid_idx + 1)
    );
    if (result) {
      lo_idx = mid_idx;
    } else {
      hi_idx = mid_idx;
    }
  }
  assertx(lo_idx + 1 == hi_idx);

  // Record the discarded and accepted layouts.
  std::copy(
    layouts.begin(), layouts.begin() + hi_idx,
    std::back_inserter(s_metadata.coloringAcceptedLayouts)
  );
  std::copy(
    layouts.begin() + hi_idx, layouts.end(),
    std::back_inserter(s_metadata.coloringDiscardedLayouts)
  );
  auto const coloring = performColoring(
    layouts.begin(),
    layouts.begin() + hi_idx
  );
  s_metadata.coloring = coloring;

  FTRACE(3, "Final coloring: {} layouts.\n", hi_idx);

  return {layouts.begin() + hi_idx, coloring};
}

void applyColoring(const ColorMap& coloring) {
  for (auto const& [key, color] : coloring) {
    const_cast<StringData*>(key)->setColor(color);
  }
}

std::string dumpColoringInfo() {
  if (!s_metadata.coloring) {
    return "Coloring unsuccessful\n\n";
  }

  std::ostringstream ss;
  {
    auto maxColor = Color{0};
    for (auto const& [key, color] : *s_metadata.coloring) {
      maxColor = std::max(maxColor, color);
    }
    ss << maxColor << " colors used\n\n";
  }

  {
    auto const& discarded = s_metadata.coloringDiscardedLayouts;
    ss << "Discarded layouts (" << discarded.size() << "):\n";
    for (auto const& [layout, _] : discarded) {
      ss << "  " << layout->describe() << "\n";
    }
    ss << "\n";
  }

  {
    ss << "Color assignments:\n";
    auto colorsToKeys = folly::F14FastMap<Color, std::vector<const StringData*>>{};
    for (auto const& [key, color] : *s_metadata.coloring) {
      colorsToKeys[color].push_back(key);
    }

    auto colorsToKeysVec =
      std::vector<std::pair<Color, std::vector<const StringData*>>>(
        colorsToKeys.begin(), colorsToKeys.end());
    std::sort(
      colorsToKeysVec.begin(), colorsToKeysVec.end(),
      [&](auto const& a, auto const& b) { return a.first < b.first; }
    );

    for (auto const& [color, keys] : colorsToKeysVec) {
      ss << "  " << color << ":\n";
      for (auto const& key : keys) {
        ss << "    " << folly::cEscape<std::string>(key->data()) << "\n";
      }
    }
    ss << "\n";
  }

  {
    ss << "Max color indices:\n";
    for (auto const& [layout, _] : s_metadata.coloringAcceptedLayouts) {
      auto const& coloring = *s_metadata.coloring;
      auto maxColor = Color{0};
      for (auto const& key : layout->keyOrder()) {
        auto const iter = coloring.find(key);
        assertx(iter != coloring.end());
        maxColor = std::max(maxColor, iter->second);
      }
      ss << "  " << layout->describe() << ": " << maxColor << "\n";
    }
    ss << "\n";
  }

  return ss.str();
}

}}
