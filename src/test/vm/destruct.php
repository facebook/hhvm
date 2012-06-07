<?php

class C {

  public function bluh() {
    global $x;
    $x = null;
  }

  public function __destruct() {
    print "In C::__destruct()\n";
  }
}

function foo() {
  $c = new C;
}

function main() {
  print "Test begin\n";

  foo();

  // Tricky case: $this is the last reference
  $x = new C;
  $x->bluh();

  print "Test end\n";
}
main();
