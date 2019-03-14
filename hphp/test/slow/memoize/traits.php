<?hh


require_once(__DIR__.'/namespacefoo.php.inc');
require_once(__DIR__.'/namespacebar.php.inc');

function basic_test(string $class, string $fn): void {
  $c = new $class();
  echo $c->$fn().' ';
  echo $c->$fn().' ';
  $c = new $class();
  echo $c->$fn().' ';
  echo $c->$fn()."\n";
}

echo "basic trait use\n";
trait T {
  private static $testIT = 0;
  <<__Memoize>>public function test() {return self::$testIT++;}
}
class C {use T;}

basic_test(C::class, 'test');

echo "override method\n";
trait TO {
  private static $testITO = 1000;
  <<__Memoize>>public function test() {return self::$testITO++;}
}
class CO {
  use TO;

  private static $testICO = 10;
  <<__Memoize>>public function test() {return self::$testICO++;}
}

basic_test(CO::class, 'test');

echo "override meth with different sig\n";
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

basic_test(CDS::class, 'test');
$c = new CDS();
echo $c->test(5).' ';
echo $c->test(5).' ';
$c = new CDS();
echo $c->test(5).' ';
echo $c->test(5)."\n";

echo "override meth with different sig with namespaces\n";
class CN {use Bar\TN;}

basic_test(CN::class, 'test');
$c = new CN();
echo $c->test(3).' ';
echo $c->test(3).' ';
$c = new CN();
echo $c->test(3).' ';
echo $c->test(3)."\n";

echo "static trait method\n";

abstract final class IncStatics {
  public static $i = 50;
}
function inc() { return IncStatics::$i++; }
trait TS {
  <<__Memoize>> public static function testTraitStatic() { return inc(); }
}
class CS1 { use TS; }
class CS2 { use TS; }

echo CS1::testTraitStatic().' ';
echo CS1::testTraitStatic().' ';
echo CS2::testTraitStatic().' ';
echo CS2::testTraitStatic();
