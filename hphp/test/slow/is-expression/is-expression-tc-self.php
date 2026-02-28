<?hh

class C {
  const type T = arraykey;
  public static function isT(mixed $x): void {
    if ($x is self::T) {
      echo "arraykey\n";
    } else {
      echo "not arraykey\n";
    }
  }
}
final class D extends C {
  const type T = int;
}
final class E extends C {
  const type T = string;
}


<<__EntryPoint>>
function main_is_expression_tc_self() :mixed{
D::isT('foo');
C::isT('foo');
E::isT('foo');
echo "\n";
D::isT(1);
C::isT(1);
E::isT(1);
echo "\n";
D::isT(1.5);
C::isT(1.5);
E::isT(1.5);
echo "\n";
D::isT(false);
C::isT(false);
E::isT(false);
echo "\n";
D::isT(fopen(__FILE__, 'r'));
C::isT(fopen(__FILE__, 'r'));
E::isT(fopen(__FILE__, 'r'));
}
