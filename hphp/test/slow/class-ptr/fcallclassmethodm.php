<?hh

class C {
  public static function m(): void { echo "hello\n"; }
}

<<__EntryPoint>>
function main(): void {
  $x = "C";
  $x::m();
  $y = C::class;
  $y::m();
}
