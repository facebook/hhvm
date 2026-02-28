<?hh

class C1 {
  private function f1(): int { return 123; }
}

class C2 extends C1 {
  public function f1(): bool { return true; }
}
class C3 extends C1 {
  public static function foo(C1 $c) :mixed{
    return $c->f1();
  }
  private function f1(): string { return "C3"; }
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(C3::foo(__hhvm_intrinsics\launder_value(new C2())));
  var_dump(C3::foo(__hhvm_intrinsics\launder_value(new C3())));
}
