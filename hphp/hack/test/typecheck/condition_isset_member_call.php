<?hh // partial
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

class C {
  private ?int $x = null;

  public function munge(): void {
    $this->x = null;
  }

  public function get(): int {
    if (isset($this->x)) {
      $this->munge();
      return $this->x;
    } else {
      return 0;
    }
  }
}
