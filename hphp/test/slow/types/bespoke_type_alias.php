<?hh

newtype Foo = shape(
  'a' => string,
  'b' => int,
);

class FooClass {
}

<<__EntryPoint>>
function main(): void {
  $ts = type_structure(__hhvm_intrinsics\launder_value('Foo'));

  var_dump($ts['kind']);
  var_dump($ts);
}
