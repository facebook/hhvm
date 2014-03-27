<?php

trait B {
  public function __construct() {
    var_dump('__construct');
  }
}
class A {
  use B;
}

function main() {
  foreach (array('A', 'B') as $class) {
    $rc = new ReflectionClass($class);
    foreach ($rc->getMethods() as $method) {
      var_dump($method->isConstructor());
    }
  }

  new A;
  $rc = new ReflectionClass('A');
  var_dump($rc->newInstance());
  var_dump($rc->newInstanceWithoutConstructor());
}
main();
