<?hh
class Foo {
  public static function bar($i) :mixed{
    echo "$i\n";
  }
}

<<__EntryPoint>>
function main_reflection_method_invoke_args_static() :mixed{
$class = new ReflectionClass('Foo');
$method = $class->getMethod('bar');
$method->invokeArgs($class, vec["Hello world"]); // works in php, not HHVM
}
