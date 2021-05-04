<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\_Private;

final class OptionalIncrementalTimeout {
  private ?int $end;
  public function __construct(
    ?int $timeout_ns,
    private (function(): ?int) $timeoutHandler,
  ) {
    if ($timeout_ns is null) {
      $this->end = null;
      return;
    }
    $this->end = self::nowNS() + $timeout_ns;
  }

  public function getRemainingNS(): ?int {
    if ($this->end is null) {
      return null;
    }

    $remaining = $this->end - self::nowNS();
    if ($remaining <= 0) {
      $th = $this->timeoutHandler;
      return $th();
    }
    return $remaining;
  }

  private static function nowNS(): int {
    /* HH_FIXME[2049] PHP stdlib */
    /* HH_FIXME[4107] PHP stdlib */
    return \clock_gettime_ns(\CLOCK_MONOTONIC);
  }
}
