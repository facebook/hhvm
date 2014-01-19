<?php
trait T {
  public static function bar(self $a1) { echo __CLASS__.
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
  public static function bar(self $a1) { echo __CLASS__.
    " accepts param from subclass ".get_class($a1)."\n"; }
}
class C extends B {
  use T;
}

class D {
  public static function test(self $a1)  { echo __CLASS__."\n"; }
}

function main() {
  $c = new C;
  B::test($c);
  C::test($c);
  D::test($c);
}

main();

