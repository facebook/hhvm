<?php
trait T {
  public static function bar(parent $a1) { echo __CLASS__.
    " accepts param from subclass ".get_class($a1)."\n"; }
}

class A {
  public static function foo($a1)      { static::bar($a1); }
  public static function bar(self $a1) { echo __CLASS__.
    " accepts param from subclass ".get_class($a1)."\n"; }
}

class B extends A {
  public static function test($a1) {
    A::foo($a1);
    self::foo($a1);
  }
  public static function bar(parent $a1) { echo __CLASS__.
    " accepts param from subclass ".get_class($a1)."\n"; }
}
class C extends B {
  use T;
}

class D extends C { }
class E extends D {
  use T;
  // public static function test(parent $a1)  { echo __CLASS__."\n"; }
}

function main() {
  $a = new A;
  $b = new B;
  $c = new C;
  B::test($a);
  C::test($b);
  E::test($c);
}

main();

