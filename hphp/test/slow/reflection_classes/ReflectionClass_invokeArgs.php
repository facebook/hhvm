<?hh

class Foo {
  public static function bar($i) :mixed{
    echo "$i\n";
  }

  public function baz($i) :mixed{
    echo "$i\n";
  }
}

function test_invoke($ref, $instance) :mixed{
  try {
    $ref->invokeArgs($instance, vec["SUCCESS"]);
  } catch (Exception $e) {
    echo get_class($e) . ': ' . $e->getMessage() . "\n";
  }
}


<<__EntryPoint>>
function main_reflection_class_invoke_args() :mixed{
$class = new ReflectionClass('Foo');
$static_method = $class->getMethod('bar');
$instance_method = $class->getMethod('baz');

test_invoke($static_method, null);
test_invoke($static_method, $class);

test_invoke($instance_method, null);
test_invoke($instance_method, $class);
}
