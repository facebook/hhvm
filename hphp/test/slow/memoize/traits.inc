<?hh

function basic_test(string $class, string $fn): void {
  $c = new $class();
  echo $c->$fn().' ';
  echo $c->$fn().' ';
  $c = new $class();
  echo $c->$fn().' ';
  echo $c->$fn()."\n";
}
trait T {
  private static $testIT = 0;
  <<__Memoize>>public function test() {$t = self::$testIT; self::$testIT++; return $t;}
}
class C {use T;}
trait TO {
  private static $testITO = 1000;
  <<__Memoize>>public function test() {$t = self::$testITO; self::$testITO++; return $t;}
}
class CO {
  use TO;

  private static $testICO = 10;
  <<__Memoize>>public function test() {$t = self::$testICO; self::$testICO++; return $t;}
}
trait TDS {

  private static $testITDS = 1000;
  <<__Memoize>>
  public function test() {$t = self::$testITDS; self::$testITDS++; return $t;}
}
class CDS{
  use TDS;

  private static $testICDS = 20;
  <<__Memoize>>public function test($a = 0) {
    $t = self::$testICDS; self::$testICDS++; return $a + $t;
  }
}
class CN {use Bar\TN;}

abstract final class IncStatics {
  public static $i = 50;
}
function inc() { $t = IncStatics::$i; IncStatics::$i++; return $t; }
trait TS {
  <<__Memoize>> public static function testTraitStatic() { return inc(); }
}
class CS1 { use TS; }
class CS2 { use TS; }
