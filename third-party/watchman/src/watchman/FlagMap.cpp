/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/FlagMap.h"
#include <string.h>
#include <algorithm>

void w_expand_flags(
    const struct flag_map* fmap,
    uint32_t flags,
    char* buf,
    size_t len) {
  bool first = true;
  *buf = '\0';
  while (fmap->label && len) {
    if ((flags & fmap->value) == fmap->value) {
      size_t space;

      if (!first) {
        *buf = ' ';
        buf++;
        len--;
      } else {
        first = false;
      }

      space = std::min(len, strlen(fmap->label) + 1);
      memcpy(buf, fmap->label, space);

      len -= space - 1;
      buf += space - 1;
    }
    fmap++;
  }
}

/* vim:ts=2:sw=2:et:
 */
