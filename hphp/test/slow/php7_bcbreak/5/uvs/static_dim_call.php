<?hh

class Foo {
  static $bar = darray['baz' => myfunc<>];
}

function myfunc() {
  return 'quux';
}
<<__EntryPoint>>
function entrypoint_static_dim_call(): void {

  error_reporting(-1);

  var_dump(Foo::$bar['baz']());
}
