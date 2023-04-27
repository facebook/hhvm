<?hh

class Foo {
  const type TFoo = shape('a' => bool, 'b' => int);
}

<<__EntryPoint>>
function main(): void {
  $ts = type_structure(__hhvm_intrinsics\launder_value(Foo::class), 'TFoo');

  var_dump($ts['kind']);
}
