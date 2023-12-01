<?hh
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
  public static function foo(): void {
    $blah = vec[3, 4, 5];
    print "Hello, $blah[0]dude"; // hphp output: Hello, 3dude
  }
}

function main(): void {
  Meh::foo();
}
