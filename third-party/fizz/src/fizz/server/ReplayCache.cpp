/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/server/ReplayCache.h>

using fizz::server::ReplayCacheResult;

namespace fizz {

folly::StringPiece toString(ReplayCacheResult result) {
  switch (result) {
    case ReplayCacheResult::NotChecked:
      return "NotChecked";
    case ReplayCacheResult::NotReplay:
      return "NotReplay";
    case ReplayCacheResult::MaybeReplay:
      return "MaybeReplay";
    case ReplayCacheResult::DefinitelyReplay:
      return "DefinitelyReplay";
  }
  return "Invalid ReplayCacheResult";
}
} // namespace fizz
