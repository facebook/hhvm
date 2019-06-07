<?hh

class Foo {
  public static function bar(...$args) {
    var_dump($args);
  }
}

class Bar {
}

function main() {
  $m = new ReflectionMethod('Foo', 'bar');
  $m->invoke($m);
  $m->invoke(new Bar());
}


<<__EntryPoint>>
function main_invokemethod_static() {
main();
}
