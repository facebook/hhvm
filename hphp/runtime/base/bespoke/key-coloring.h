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

#include "hphp/runtime/base/bespoke/layout-selection.h"

#include <folly/container/F14Map.h>

namespace HPHP::bespoke {

using Color = uint16_t;
using ColorMap = folly::F14FastMap<const StringData*, Color>;

struct ColoringMetaData {
  ColorMap coloring;
  size_t numColoredFields;
};

// Finds a coloring for the a fixed-length prefix of the fields
// of the supplied struct layouts. The coloring guarantees that no two fields
// of a layout have the same color if both of them are within the prefix length.
// None of the fields within the prefix length may have
// StringData::kInvalidColor or StringData::kDupColor. Fields after this prefix
// length have a valid color, but it is not guaranteed to be unique within the
// layout.
//
// The exact length of the prefix to be colored is determined by a
// binary search in the range [1, kMaxColor - 2].
// There is always a valid assignment for kMaxColor > 2.
//
// This function returns the color map and the maximum number of fields colored.
ColoringMetaData findKeyColoring(LayoutWeightVector& layouts);

// Assign colors to the strings that are fields of the layouts. All fields that
// are not colored by findKeyColoring are assigned StringData::kDupColor.
void applyColoring(const ColoringMetaData& coloring,
                   const LayoutWeightVector& layouts);

std::string dumpColoringInfo();

}
