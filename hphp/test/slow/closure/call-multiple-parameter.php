<?php
class Foo {
  private $x = 3;
}
$foo = new Foo;
$foobar = function ($add, $mult, $prefix) {
  $msg = $prefix . ": " . strval(($this->x + $add) * $mult);
  var_dump($msg);
};
// The call method does not take a scope parameter.
// Instead, it will always use the class of the object as its scope.
// We are passing multiple parameters to the closure call
$foobar->call($foo, 3, 2, "Info"); // prints string(8) "Info: 12"
