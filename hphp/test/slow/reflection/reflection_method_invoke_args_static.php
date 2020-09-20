<?hh
class Foo {
  public static function bar($i) {
    echo "$i\n";
  }
}

<<__EntryPoint>>
function main_reflection_method_invoke_args_static() {
$class = new ReflectionClass('Foo');
$method = $class->getMethod('bar');
$method->invokeArgs($class, varray["Hello world"]); // works in php, not HHVM
}
