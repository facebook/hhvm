<?php

print "Test begin\n";

interface I {
  public static function foo();
}
class C implements I {
  public static function foo($x) {
    echo 'Hello ' . $x . "\n";
  }
}
C::foo("5");

print "Test end\n";
