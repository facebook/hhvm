<?hh

class C {
  public static function isThis(mixed $x): void {
    if ($x is this) {
      echo "this\n";
    } else {
      echo "not this\n";
    }
  }
}

final class D extends C {}

C::isThis(new C());
C::isThis(new D());
C::isThis(new stdClass());
C::isThis(null);
C::isThis(true);
C::isThis(1.5);
C::isThis(1);
C::isThis(STDIN);

echo "\n";

D::isThis(new C());
D::isThis(new D());
D::isThis(new stdClass());
D::isThis(null);
D::isThis(true);
D::isThis(1.5);
D::isThis(1);
D::isThis(STDIN);
