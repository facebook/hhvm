<?php

class Foo {
  public static function bar($i) {
    echo "$i\n";
  }

  public function baz($i) {
    echo "$i\n";
  }
}

function test_invoke($ref, $instance) {
  try {
    $ref->invokeArgs($instance, ["SUCCESS"]);
  } catch (Exception $e) {
    echo get_class($e) . ': ' . $e->getMessage() . "\n";
  }
}

$class = new ReflectionClass('Foo');
$static_method = $class->getMethod('bar');
$instance_method = $class->getMethod('baz');

test_invoke($static_method, null);
test_invoke($static_method, $class);

test_invoke($instance_method, null);
test_invoke($instance_method, $class);
