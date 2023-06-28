<?hh

interface I1 {
  public function baz():mixed;
}
interface I2 {}

class C1 implements I1 {
  public function baz(): int { return 1; }
}
class C2 implements I1 {
  public function baz(): int { return 2; }
}

abstract class C3 implements I1, I2 {
  public function baz(): string { return "3"; }
}
class C4 implements I2 {}
class C5 implements I2 {}

function bar2(): I2 {
  if (__hhvm_intrinsics\launder_value(true)) {
    return new C4();
  } else {
    return new C3();
  }
}

function bar() :mixed{ return bar2(); }

function foo() :mixed{
  $x = bar() as I1;
  return $x->baz();
}

<<__EntryPoint>> function main() :mixed{
  try {
    var_dump(foo());
  } catch (Exception $e) {
    echo "failed\n";
  }
}
