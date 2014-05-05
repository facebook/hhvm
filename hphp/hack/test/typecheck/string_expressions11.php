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

class Meh {
  public static string $six = "blah";

  public static function foo(): void {
    $blah = "uh oh";
    print "Hello, ${Meh::$six}"; // hphp output: Hello, uh oh
  }
}

Meh::foo();
