<?php
class A {
  public static function testNotMemoized() { static $i = 100; return $i++; }
  <<__Memoize>>
  public static function testStatic() { static $i = 110; return $i++; }
}

echo A::testNotMemoized().' ';
echo A::testNotMemoized().' ';
echo A::testStatic().' ';
echo A::testStatic()."\n";
