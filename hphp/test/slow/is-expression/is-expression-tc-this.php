<?hh

class C {
  const type T = this;
  public static function isT(mixed $x): void {
    if ($x is this::T) {
      echo static::class."\n";
    } else {
      echo "not ".static::class."\n";
    }
  }
}
final class D extends C {}
final class E extends C {}


<<__EntryPoint>>
function main_is_expression_tc_this() :mixed{
$d = new D();
D::isT($d);
C::isT($d);
E::isT($d);
echo "\n";
$s = new stdClass();
D::isT($s);
C::isT($s);
E::isT($s);
echo "\n";
$e = new E();
D::isT($e);
C::isT($e);
E::isT($e);
echo "\n";
$c = new C();
D::isT($c);
C::isT($c);
E::isT($c);
}
