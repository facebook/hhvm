<?hh

interface I1 {
  public function f1(): int;
}
interface I2 {}
interface I3 {}

class C1 implements I1, I2, I3 {
  public function f1(): int { return 1; }
}
class C2 implements I1, I2, I3 {
  public function f1(): int { return 2; }
}

class C3 implements I1 {
  public function f1(): int { return 3; }
}
class C4 implements I2 {}

abstract class C5 implements I3 {
  public function f1(): string { return "5"; }
}

function func1() :mixed{
  if (__hhvm_intrinsics\launder_value(true)) return new C1();
  return new C2();
}

function func2() :mixed{
  $x = func1();
  if ($x is I1) return $x->f1();
  return 0;
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(func2());
}
