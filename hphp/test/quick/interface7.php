<?php

print "Test begin\n";

interface I {
  public function foo($x, $y);
}
class C implements I {
  public function foo($x) {
    echo 'Hello ' . $x . "\n";
  }
}
$o = new C;
$o->foo("5");

print "Test end\n";
