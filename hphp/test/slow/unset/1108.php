<?hh

function test() {
  unset(Unset1108::$a[0]);
  var_dump(Unset1108::$a);
}

abstract final class Unset1108 {
  public static $a;
}

<<__EntryPoint>> function main(): void {
Unset1108::$a = varray[10];
var_dump(Unset1108::$a);
test();
var_dump(Unset1108::$a);
}
