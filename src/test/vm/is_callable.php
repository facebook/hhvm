<?php

function f() {}
// user function
var_dump(is_callable('f'));
// builtin function
var_dump(is_callable('array_merge'));
// nonexistent function
var_dump(is_callable('asdfjkl'));
echo "\n";

class C {
  public static function foo() {}
  private static function bar() {}
}
// user public static method
var_dump(is_callable(array('C','foo')));
// user private static method
var_dump(is_callable(array('C','bar')));
// nonexistent static method
var_dump(is_callable(array('C','asdfjkl')));
echo "\n";

class D {
  public function foo() {}
  private function bar() {}
}
$obj = new D;
// user public instance method
var_dump(is_callable(array($obj,'foo')));
// user private instance method
var_dump(is_callable(array($obj,'bar')));
// nonexistent instance method
var_dump(is_callable(array($obj,'asdfjkl')));

