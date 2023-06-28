<?hh

interface I {}

class C1 implements I {
  public static function foo() :mixed{ return 1; }
}
class C2 implements I {
  public static function foo() :mixed{ return 3; }
}
abstract class C3 implements I {
  private static function foo($a) :mixed{ return 2; }
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(C1::foo());
}
