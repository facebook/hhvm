<?php

class Foo {
  public $x;
  public function __construct($x) { $this->x = $x; }
}

$a = new Foo(42);

$gotException = false;
try {
  $b = new ReflectionProperty(null, null);
} catch (ReflectionException $ex) {
  $gotException = true;
}
var_dump($gotException);

$gotException = false;
try {
  $b = new ReflectionProperty(null, 'x');
} catch (ReflectionException $ex) {
  $gotException = true;
}
var_dump($gotException);

$gotException = false;
try {
  $b = new ReflectionProperty($a, null);
} catch (ReflectionException $ex) {
  $gotException = true;
}
var_dump($gotException);

$b = new ReflectionProperty($a, 'x');
var_dump($b instanceof ReflectionProperty);
