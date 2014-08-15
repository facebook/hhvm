<?php
class A {
  <<__Memoize>>
  public function &testRefReturn() { return array(1,2,3); }
}

echo (new A())->testRefReturn();
