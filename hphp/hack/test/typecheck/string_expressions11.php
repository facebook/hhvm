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

class Meh {
  public static string $six = "blah";

  public static function foo(): void {
    $blah = "uh oh";
    print "Hello, ${Meh::$six}"; // hphp output: Hello, uh oh
  }
}

function main(): void {
  Meh::foo();
}
