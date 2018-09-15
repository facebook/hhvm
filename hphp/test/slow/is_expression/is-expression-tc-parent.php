<?hh // strict

abstract class C {
  const type T as arraykey = arraykey;
}
final class D extends C {
  const type T = int;
  public static function isT(mixed $x): void {
    if ($x is parent::T) {
      echo "arraykey\n";
    } else {
      echo "not arraykey\n";
    }
  }
}
final class E extends C {
  const type T = string;
  public static function isT(mixed $x): void {
    if ($x is parent::T) {
      echo "arraykey\n";
    } else {
      echo "not arraykey\n";
    }
  }
}


<<__EntryPoint>>
function main_is_expression_tc_parent() {
D::isT('foo');
E::isT('foo');
echo "\n";
D::isT(1);
E::isT(1);
echo "\n";
D::isT(1.5);
E::isT(1.5);
echo "\n";
D::isT(false);
E::isT(false);
echo "\n";
D::isT(STDIN);
E::isT(STDIN);
}
