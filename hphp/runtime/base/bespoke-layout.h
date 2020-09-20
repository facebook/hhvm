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

#ifndef HPHP_BESPOKE_LAYOUT_H_
#define HPHP_BESPOKE_LAYOUT_H_

#include <cstdint>
#include <string>

#include "hphp/util/assertions.h"

namespace HPHP {

namespace bespoke { struct Layout; }

/*
 * Identifies information about a bespoke layout necessary to JIT code handling
 * arrays of that layout
 */
struct BespokeLayout {
  explicit BespokeLayout(const bespoke::Layout* layout)
    : m_layout(layout) {
    assertx(layout);
  }

  bool operator==(const BespokeLayout& o) const {
    return o.m_layout == m_layout;
  }
  bool operator!=(const BespokeLayout& o) const {
    return !(*this == o);
  }

  /* get the index of this layout */
  uint16_t index() const;

  /* retrieve a layout by index */
  static BespokeLayout LayoutFromIndex(uint16_t idx);

  /* get a human-readable string describing the layout */
  std::string describe() const ;

private:
  const bespoke::Layout* m_layout{nullptr};
};

} // namespace HPHP

#endif // HPHP_BESPOKE_LAYOUT_H_



