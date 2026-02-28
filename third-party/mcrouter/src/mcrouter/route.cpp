/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "route.h"

#include <memory>

#include "mcrouter/lib/fbi/cpp/util.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

static bool match_pattern_helper(
    const char* pi,
    const char* pend,
    const char* ri,
    const char* rend) {
  while (pi != pend) {
    if (ri == rend) {
      return false;
    } else if (*pi == '*') {
      /* Swallow multiple stars */
      do {
        ++pi;
      } while (pi != pend && *pi == '*');

      if (pi == pend) {
        /* Terminating star matches any string not containing slashes */
        return memchr(ri, '/', rend - ri) == nullptr;
      } else if (*pi == '/') {
        /* start + slash: advance ri to the next slash */
        ri = static_cast<const char*>(memchr(ri, '/', rend - ri));
        if (ri == nullptr) {
          return false;
        }
      } else {
        /* Try to match '*' with every prefix,
           recursively try to match the remainder */
        while (ri != rend && *ri != '/') {
          if (match_pattern_helper(pi, pend, ri, rend)) {
            return true;
          }
          ++ri;
        }
        return false;
      }
    } else if (*pi++ != *ri++) {
      return false;
    }
  }

  return ri == rend;
}

/**
 * True if pattern (like "/foo/a*c/") matches a route (like "/foo/abc")
 */
bool match_pattern_route(folly::StringPiece pattern, folly::StringPiece route) {
  return match_pattern_helper(
      pattern.begin(), pattern.end(), route.begin(), route.end());
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
