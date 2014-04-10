<?php

interface TestInterface {
  public function __construct();
}

abstract class TestClassImplementingInterface implements TestInterface {
}

class TestConcreteClass {
  public function __construct() { }
}

abstract class TestAbstractClass{
  abstract public function __construct();
}

trait TestTrait {
  abstract public function __construct();
}

function main() {
  $classes = array(
    'TestClassImplementingInterface', // false
    'TestInterface', // false
    'TestConcreteClass', // true
    'TestAbstractClass', // true
    'TestTrait' // true
  );

  $out = array();
  foreach ($classes as $class) {
    $rc = (new ReflectionClass($class));
    $out[$class] = $rc->getMethod('__construct')->isConstructor();
  }

  ksort($out);
  var_dump($out);
}

main();
