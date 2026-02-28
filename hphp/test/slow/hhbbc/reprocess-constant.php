<?hh

function foo(): string {
  return __hhvm_intrinsics\launder_value("aaa");
}

enum class B: string {
  string STR = foo();
}

class A {
  const STR = 'bbbb';
  const SHAPE = shape(
    'k1' => shape(
      'k2' => B::STR,
      'k3' => shape('k4' => self::STR),
    )
  );
}

<<__EntryPoint>>
function main() {
  var_dump(A::SHAPE);
}
