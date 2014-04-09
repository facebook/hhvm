<?php

class Foo {
  public static function bar() {
    echo get_called_class()."\n";
  }
  public function boo($var) {
    var_dump($var);
    echo get_called_class()."\n";
 }
}
class Baz extends Foo { }

$class = new ReflectionClass('Baz');
$bar_method = $class->getMethod('bar');
$bar_method->invoke(null); // the correct answer is 'Baz'
$boo_method = $class->getMethod('boo');
$boo_method->invokeArgs(new Baz(), [true]);

$standalone_bar = new ReflectionMethod('Baz', 'bar');
$standalone_bar->invoke(null);
$standalone_boo = new ReflectionMethod('Baz', 'boo');
$standalone_boo->invokeArgs(new Baz(), [true]);
