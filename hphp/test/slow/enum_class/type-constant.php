<?hh

class Bar {}

type SomeType3 = Bar;
type SomeType2 = SomeType3;
type SomeType = SomeType2;

function func(): mixed {
  return __hhvm_intrinsics\launder_value(new Bar());
}

enum class Foo : Bar {
  Bar A = func() as SomeType;
  Bar B = func() as SomeType;
}

<<__EntryPoint>>
function main() {
  var_dump(Foo::A);
}
