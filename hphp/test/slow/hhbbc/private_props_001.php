<?php

/*
 * Test inference handles things which have side effects that could
 * change property types.
 *
 * Currently private properties get a control-flow insensitive type
 * (same type at all program points) so this should work.
 */

class Lol {}

function breakThings(Foo $x) { $x->nullOther(); }

class Foo {
  private $other;

  public function __construct() {
    $this->other = new Lol;
    breakThings($this);
    if (is_null($this->other)) {
      echo "was null\n";
    } else {
      echo "something broke\n";
    }
  }

  public function nullOther() {
    $this->other = null;
  }
}

function main() {
  $x = new Foo();
}

main();
