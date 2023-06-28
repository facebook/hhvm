<?hh

abstract class C1 {
  public function foo() :mixed{
    $x = $this->f1();
    if ($x is int) return "int";
    return "string";
  }
  private function f1(): int { return 123; }
}

class C2 extends C1 {
  public function f1(): string { return "C2"; }
}
class C3 extends C1 {
  public function f1(): string { return "C3"; }
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(__hhvm_intrinsics\launder_value(new C2())->foo());
}
