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

namespace HPHP::bespoke {

TRACE_SET_MOD(bespoke);

//////////////////////////////////////////////////////////////////////////////

namespace {

// For logging
ColoringMetaData s_metadata;

//////////////////////////////////////////////////////////////////////////////

Optional<ColorMap> performColoring(const LayoutWeightVector& layouts,
                                   size_t maxColored) {
  FTRACE(
    3,
    "Attempting to color {} layouts with {} distinctly colored fields.\n",
    layouts.size(), maxColored);

  assertx(maxColored > 0);

  // 1. Construct the interference graph from the layout set.
  using VertexSet = folly::F14FastSet<const StringData*>;
  using EdgeSet = folly::F14FastMap<const StringData*, VertexSet>;
  // Two colors are reserved for invalid and duplicate colors
  assertx(maxColored <= StructLayout::kMaxColor - 2);
  auto allEdges = [&] {
    auto result = EdgeSet();
    std::for_each(layouts.cbegin(), layouts.end(), [&](auto const& pair) {
      auto const& layout = pair.first;
      auto const numFieldsToColor = std::min(layout->numFields(), maxColored);
      for (Slot i = 0; i < numFieldsToColor; ++i) {
        auto const first = layout->field(i).key;
        result.try_emplace(first);
        for (Slot j = i + 1; j < numFieldsToColor; ++j) {
          auto const second = layout->field(j).key;
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
    std::stable_sort(
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
      auto constexpr firstColor = StringData::kDupColor + 1;
      for (Color i = firstColor; i <= StructLayout::kMaxColor; i++) {
        if (colorsUsed.find(i) == colorsUsed.end()) {
          colors[vertex] = i;
          return true;
        }
      }
      return false;
    }();
    if (!colorAssigned) {
      FTRACE(
        3,
        "Colors exhausted while coloring {} layouts for {} fields.\n",
         layouts.size(), maxColored);
      return std::nullopt;
    }
  }

  // 4. Validate the coloring.
  if constexpr (debug) {
    std::for_each(layouts.cbegin(), layouts.cend(), [&](auto const& pair) {
      auto const& layout = pair.first;
      auto const numFieldsToColor = std::min(layout->numFields(), maxColored);
      for (Slot i = 0; i < numFieldsToColor; ++i) {
        auto const first = layout->field(i).key;
        always_assert(
          StringData::kDupColor < colors[first] &&
          colors[first] <= StructLayout::kMaxColor
        );
        for (Slot j = i + 1; j < numFieldsToColor; ++j) {
          auto const second = layout->field(j).key;
          always_assert(colors[first] != colors[second]);
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
    FTRACE(3, "Colored {} layouts with {} colors.\n", layouts.size(),
           maxColor->second);
  }

  return {colors};
}
}

//////////////////////////////////////////////////////////////////////////////

ColoringMetaData findKeyColoring(LayoutWeightVector& layouts) {
  FTRACE(3, "Attempting to color a prefix of {} layouts.\n", layouts.size());
  if (layouts.empty()) return {{}, 1};

  // Sort the layouts in descending order by weight.
  std::sort(
    layouts.begin(), layouts.end(),
    [](auto const& a, auto const& b) { return a.second > b.second; }
  );

  // Binary search for max number of distinctly colorable fields.
  // performColoring is non-deterministic because it uses F14 maps and sets,
  // so it should not be called more than once for same input. The greedy
  // algorithm is also non-monotonic, so the search might return a local optima,
  // but that is acceptable.
  size_t lastHigh = StructLayout::kMaxColor - 1; // lowest unsuccessful so far
  size_t lastLow = 1; // highest successful so far
  Optional<ColorMap> result;
  while (lastLow + 1 < lastHigh) {
    auto const mid = (lastLow + lastHigh) / 2;
    auto res = performColoring(layouts, mid);
    if (res) {
      lastLow = mid;
      result = std::move(res);
    } else {
      lastHigh = mid;
    }
  }
  assertx(lastLow + 1 == lastHigh);
  if (!result) {
    assertx(lastLow == 1);
    // Coloring always succeeds when there is just one field to color in each
    // layout.
    result = performColoring(layouts, 1);
  }
  assertx(result);

  s_metadata.coloring = *result;
  s_metadata.numColoredFields = lastLow;

  FTRACE(3, "Final coloring: {} max colored fields.\n", lastLow);

  return {*result, lastLow};
}

void applyColoring(const ColoringMetaData& coloring,
                   const LayoutWeightVector& layouts) {
  for (auto const& [key, color] : coloring.coloring) {
    const_cast<StringData*>(key)->setColor(color);
  }
  std::for_each(layouts.cbegin(), layouts.cend(), [&](auto const& pair) {
    auto const& layout = pair.first;
    for (Slot i = coloring.numColoredFields; i < layout->numFields(); ++i){
      auto const key = layout->field(i).key.get();
      if (key->color() == StringData::kInvalidColor) {
        const_cast<StringData*>(key)->setColor(StringData::kDupColor);
      }
    }
  });
  StructLayout::setMaxColoredFields(coloring.numColoredFields);
}

std::string dumpColoringInfo() {
  std::ostringstream ss;
  ss << "Max number of colored fields: " << s_metadata.numColoredFields << "\n";
  {
    auto maxColor = Color{0};
    for (auto const& [key, color] : s_metadata.coloring) {
      maxColor = std::max(maxColor, color);
    }
    ss << maxColor - 1 << " distinct colors used\n\n";
  }

  {
    ss << "Color assignments:\n";
    auto colorsToKeys = folly::F14FastMap<Color, std::vector<const StringData*>>{};
    for (auto const& [key, color] : s_metadata.coloring) {
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

    auto numStructs = size_t{0};
    eachLayout([&](Layout& l) {
      if (!l.isConcrete() || !jit::ArrayLayout(&l).is_struct()) return;
      numStructs++;
      auto const layout = StructLayout::As(&l);
      auto maxColor = Color{0};
      auto const numColoredFields =
        std::min(layout->numFields(), s_metadata.numColoredFields);
      for (Slot i = 0; i < numColoredFields; ++i) {
        auto const key = layout->field(i).key;
        auto const iter = s_metadata.coloring.find(key);
        assertx(iter != s_metadata.coloring.end());
        maxColor = std::max(maxColor, iter->second);
      }
      ss << "  " << layout->describe() << ": " << maxColor << "\n";
    });
    ss << "\n";

    ss << "Number of colored layouts: " << numStructs << "\n";
  }

  return ss.str();
}

}
