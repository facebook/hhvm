<?hh

interface I {}

class C1 implements I {
  public static function foo() { return 1; }
}
class C2 implements I {
  public static function foo() { return 3; }
}
abstract class C3 implements I {
  private static function foo($a) { return 2; }
}

<<__EntryPoint>>
function main() {
  var_dump(C1::foo());
}
