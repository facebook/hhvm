<?hh

abstract class C1 {
  public function f1(): int { return __hhvm_intrinsics\launder_value(123); }
}

class C2 extends C1 {}
class C3 extends C2 {}
class C4 extends C2 {}
class C5 extends C1 {
  public function f1(): string { return __hhvm_intrinsics\launder_value("abc"); }
}

function func2(): C1 {
  if (__hhvm_intrinsics\launder_value(true)) return new C3();
  return new C4();
}

function func1() :mixed{
  $x = func2();
  return $x->f1();
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(func1());
}
