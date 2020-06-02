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
  <<__Memoize>>public function test() {return self::$testIT++;}
}
class C {use T;}
trait TO {
  private static $testITO = 1000;
  <<__Memoize>>public function test() {return self::$testITO++;}
}
class CO {
  use TO;

  private static $testICO = 10;
  <<__Memoize>>public function test() {return self::$testICO++;}
}
trait TDS {

  private static $testITDS = 1000;
  <<__Memoize>>
  public function test() {return self::$testITDS++;}
}
class CDS{
  use TDS;

  private static $testICDS = 20;
  <<__Memoize>>public function test($a = 0) {
    return $a + self::$testICDS++;
  }
}
class CN {use Bar\TN;}

abstract final class IncStatics {
  public static $i = 50;
}
function inc() { return IncStatics::$i++; }
trait TS {
  <<__Memoize>> public static function testTraitStatic() { return inc(); }
}
class CS1 { use TS; }
class CS2 { use TS; }
