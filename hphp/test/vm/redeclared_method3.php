<?php
interface I {
  function foo();
}
class C implements I {
  static function foo() { echo "foo\n"; }
}
$obj = new C;
$obj->foo();
