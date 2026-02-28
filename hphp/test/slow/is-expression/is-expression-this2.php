<?hh

trait T {
  public static function isThis(mixed $x): void {
    if ($x is this) {
      echo "this\n";
    } else {
      echo "not this\n";
    }
  }
}

class C {
  use T;
}

final class D extends C {}


<<__EntryPoint>>
function main_is_expression_this2() :mixed{
C::isThis(new C());
C::isThis(new D());
C::isThis(new stdClass());
C::isThis(null);
C::isThis(true);
C::isThis(1.5);
C::isThis(1);
C::isThis(fopen(__FILE__, 'r'));

echo "\n";

D::isThis(new C());
D::isThis(new D());
D::isThis(new stdClass());
D::isThis(null);
D::isThis(true);
D::isThis(1.5);
D::isThis(1);
D::isThis(fopen(__FILE__, 'r'));

echo "\n";

T::isThis(new C());
T::isThis(new D());
}
