<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

class ToS {
  private ?int $i;

  public function __toString(): string {
    return 'I have no idea why anyone would want to do this, but people do.';
  }

  public function f(): string {
    if ($this->i === null) {}
    return 'x' . $this;
  }
}
