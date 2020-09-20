<?hh // strict
class C {
  public function foo(): D {
    return new D();
  }
}
class D {}
function test(?C $c): D {
  return $c?->foo();
}
function main() {
  $c = new C();
  var_dump(test($c));
  var_dump(test(null));
}

<<__EntryPoint>>
function main_nullsafe_call_2() {
main();
}
