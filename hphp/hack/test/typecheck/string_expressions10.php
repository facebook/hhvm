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

class Blah {
  public int $wut = 0;
}

class Meh {

  private Blah $dude;

  public function __construct() {
    $this->dude = new Blah();
  }

  public static function foo(?Vector<Meh> $x): void {
    $m = new Meh();
    print "Hello, {$m->dude->wt}";
    // prints "Hello, hello" in HPHP, but we disallow
    // it because we don't like interpolated
    // nested expressions
  }
}

Meh::foo(null);
