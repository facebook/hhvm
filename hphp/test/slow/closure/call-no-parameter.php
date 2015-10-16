<?php
class Foo {
  private $x = 3;
}
$foo = new Foo;
$foobar = function () {
  var_dump($this->x);
};
// The call method does not take a scope parameter.
// Instead, it will always use the class of the object as its scope.
// We are not passing a parameter to the closure call.
$foobar->call($foo); // prints int(3)
