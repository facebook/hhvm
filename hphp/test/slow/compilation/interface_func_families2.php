<?hh

interface I1 {
  public function f1(): int;
}

interface I2 {
  public function f2(): string;
}

class C1 implements I1, I2 {
  public function f1(): int { return 1; }
  public function f2(): string { return "1"; }
}
class C2 implements I1, I2 {
  public function f1(): int { return 2; }
  public function f2(): string { return "2"; }
}
class C3 implements I1 {
  public function f1(): int { return 3; }
}

class C4 {
  public function f1(): string { return "123"; }
}

function foo2(): I1 {
  if (__hhvm_intrinsics\launder_value(true)) {
    return new C1();
  }
  return new C2();
}

function foo1() :mixed{
  $x = foo2();
  return $x->f1();
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(foo1());
}
