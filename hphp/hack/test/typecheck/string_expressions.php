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

class Meh {
  private int $dude = 5;

  public static function foo(): void {
    $x = new Meh();
    print "Hello, {$x->dude}"; // hphp output: Hello, 5
  }
}

Meh::foo();
