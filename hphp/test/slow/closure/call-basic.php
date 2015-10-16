<?php
class Foo {
  private $x = 3;
}
$foo = new Foo;
$foobar = function ($add) {
  var_dump($this->x + $add);
};
// The call method does not take a scope parameter.
// Instead, it will always use the class of the object as its scope.
$foobar->call($foo, 4); // prints int(7)
