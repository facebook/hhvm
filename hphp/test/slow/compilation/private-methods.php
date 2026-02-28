<?hh

class C1 {
  public static function foo(C2 $c) :mixed{
    $x = $c->f1();
    if ($x is int) return "int";
    return "string";
  }
  private function f1(): int { return 123; }
}

class C2 extends C1 {
  private function f1(): string { return "C2"; }
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(C2::foo(__hhvm_intrinsics\launder_value(new C2())));
}
