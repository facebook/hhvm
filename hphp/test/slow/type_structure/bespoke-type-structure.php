<?hh

class Foo {
  const type FooShape = shape('a' => bool, 'b' => int);
  const type FooBool = bool;
  const type FooMixed = mixed;
}

newtype FooInt = int;

<<__EntryPoint>>
function main(): void {
  $foo = __hhvm_intrinsics\launder_value(Foo::class);

  $ts = type_structure($foo, 'FooShape');
  var_dump($ts['kind']);

  $ts = type_structure($foo, 'FooBool');
  var_dump($ts['kind']);

  $ts = type_structure($foo, 'FooMixed');
  var_dump($ts['kind']);

  $ts = type_structure(__hhvm_intrinsics\launder_value('FooInt'));
  var_dump($ts['kind']);
}
