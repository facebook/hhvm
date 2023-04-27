<?hh

class Foo {
  public static $bar = darray['baz' => myfunc<>];
}

function myfunc() {
  return 'quux';
}
<<__EntryPoint>>
function entrypoint_static_dim_call(): void {

  error_reporting(-1);
  try {
    var_dump(Foo::$bar['baz']());
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}
