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

trait TR1 {}

trait TR2 {
  require extends TR1;

  public function foo(): void {
    return parent::foo();
  }
}
