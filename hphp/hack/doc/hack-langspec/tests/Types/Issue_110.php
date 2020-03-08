<?hh // strict

namespace Issue_110;

class C {
  public static classname<C> $p1 = C::class;
  public static string $str = 'Issue_110\C';

  public static function sf1(): void {
    var_dump(C::$str == C::$p1);	// value-equality
  }
  public static function sf2(): void {
    var_dump(C::$str === C::$p1);	// same-type-and-value-equality
  }
}

function main(): void {
  C::sf1();
  C::sf2();
}

/* HH_FIXME[1002] call to main in strict*/
main();