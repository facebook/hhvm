<?php

class Foo {
  public function __construct() {}
  public function method() {}
  public function __toString() { throw new Exception('No string casts');}
}

$instance = new Foo();

// no exception ... caller-responsible for the object
$b = new ReflectionMethod(null, null);
var_dump($b instanceof ReflectionMethod);

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
