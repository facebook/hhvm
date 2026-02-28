<?hh

class Foo {
  <<__DynamicallyCallable>>
  public static function bar(...$args) :mixed{
    var_dump($args);
  }
}

class Bar {
}

function main() :mixed{
  $m = new ReflectionMethod('Foo', 'bar');
  $m->invoke($m);
  $m->invoke(new Bar());
}


<<__EntryPoint>>
function main_invokemethod_static() :mixed{
main();
}
