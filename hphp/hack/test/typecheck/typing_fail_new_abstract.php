<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

abstract class Meh {

  public function blah(): void {
    print "Why can't I be abstact too?";
  }
}

function magic_meh_builder(): void {
  $x = new Meh();
}
