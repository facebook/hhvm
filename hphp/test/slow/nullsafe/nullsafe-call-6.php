<?hh
class C {
  public function foo(): D {
    return new D();
  }
}
class D {}
function test(C $c): D {
  return $c?->foo();
}
function main() :mixed{
  $c = new C();
  var_dump(test($c));
  echo "Done\n";
}

<<__EntryPoint>>
function main_nullsafe_call_6() :mixed{
main();
}
