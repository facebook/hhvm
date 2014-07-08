<?php

class Foo {
  public function __construct() {}
  public function method() {}
  public function __toString() { throw new Exception('No string casts');}
}

$instance = new Foo();

try {
  $b = new ReflectionMethod(null, 'noSuchMethod');
} catch (ReflectionException $ex) {
  echo 'ReflectionException: ', $ex->getMessage(), "\n";
}

try {
  $b = new ReflectionMethod('Foo', 'noSuchMethod');
} catch (ReflectionException $ex) {
  echo 'ReflectionException: ', $ex->getMessage(), "\n";
}

try {
  $b = new ReflectionMethod($instance, 'noSuchMethod');
} catch (ReflectionException $ex) {
  echo 'ReflectionException: ', $ex->getMessage(), "\n";
}

$b = new ReflectionMethod($instance, 'method');
var_dump($b instanceof ReflectionMethod);
$b = new ReflectionMethod('Foo', 'method');
var_dump($b instanceof ReflectionMethod);

// Look for method 'method' in class 'Foo'
var_dump((new ReflectionMethod('Foo', 'method'))->getName());
var_dump((new ReflectionMethod(new Foo(), 'method'))->getName());
var_dump((new ReflectionMethod('Foo::method'))->getName());
// Look for method '' in class 'Foo'
try {
  var_dump((new ReflectionMethod('Foo', null))->getName());
} catch (ReflectionException $ex) {
  echo 'ReflectionException: ', $ex->getMessage(), "\n";
}
