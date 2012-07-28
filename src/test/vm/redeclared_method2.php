<?php
interface I {
  static function foo();
}
class C implements I {
  function foo() { echo "foo\n"; }
}
$obj = new C;
$obj->foo();
