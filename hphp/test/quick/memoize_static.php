<?php
class A {
  <<__Memoize>>
  public static function testStatic() { static $i = 100; return $i++; }
}

echo A::testStatic();
