<?php

class Thing {
  public static function testOne() { echo "one\n"; }
  public static function testTwo() { echo "two\n"; }
  public static function testThree() { echo "three\n"; }
  public static function testFour() { echo "four\n"; }
}


$class = new ReflectionClass('Thing');
$methods = $class->getMethods();
foreach ($methods as $method) {
  var_dump($method->getName());
  $method->invoke(null);
}
