<?php
class Foo {
  public static function bar($i) {
    echo "$i\n";
  }
}
$class = new ReflectionClass('Foo');
$method = $class->getMethod('bar');
$method->invokeArgs($class, ["Hello world"]); // works in php, not HHVM
