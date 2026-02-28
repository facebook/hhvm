<?hh

class C {
  public static function f(): void { echo "bye\n"; }
}

<<__EntryPoint>>
function main(): void {
  $x = HH\classname_to_class("C");
  echo "hello\n";
  $x::f();

  $y = HH\classname_to_class("C");
  $y::f();
}
