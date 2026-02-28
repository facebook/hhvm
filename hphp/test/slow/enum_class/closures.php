<?hh

class Bar {
  public function __construct(mixed $x) {
    __hhvm_intrinsics\launder_value($x)(true);
  }
}

enum class Foo : Bar {
  Bar A = new Bar(($x) ==> $x);
  Bar B = new Bar(($x) ==> !$x);
}

<<__EntryPoint>>
function main() {
  var_dump(Foo::A);
}
