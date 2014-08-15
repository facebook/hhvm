<?php
class A {
  <<__Memoize>>
  public function testArgs($a) { return $a; }
}

echo (new A())->testArgs(1);
