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

#include "hphp/runtime/base/bespoke/layout-selection.h"

#include <folly/container/F14Map.h>

namespace HPHP { namespace bespoke {

using Color = uint16_t;
using ColorMap = folly::F14FastMap<const StringData*, Color>;

// Finds a colorable subset of the supplied layout/weight pairs. The return
// value is a a pair of:
//
//   1) an iterator `ret` such that the range [begin, ret) is colorable
//   2) the coloring produced (if any)
//
// Like std::remove, it partitions the vector into two pieces: a colorable
// prefix, and a set of layouts that could not be colored. The return value is
// an iterator pointing to the first element of the uncolorable suffix.
//
// The current implementation sorts the layouts by weight and finds a colorable
// prefix of the resulting order.
std::pair<LayoutWeightVector::const_iterator, Optional<ColorMap>>
  findKeyColoring(LayoutWeightVector& layouts);

void applyColoring(const ColorMap& coloring);

std::string dumpColoringInfo();

}}
