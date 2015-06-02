<?php

// Test that concat binops with un-used results are not incorrectly optimized
// away if they can invoke side-effects. See task #3725260.

class A {
  public function __toString() { echo "__toString()\n"; return "heh"; }
}

$a = new A();
$b = new A();
$a . $b;

echo "done\n";
