<?hh

trait T {
  public static function isThis(mixed $x): void {
    if ($x is classname<this>) {
      echo "this\n";
    } else {
      echo "not this\n";
    }
  }
}

abstract class C {
  use T;
}

final class D extends C {}

C::isThis(C::class);
C::isThis(D::class);
C::isThis(stdClass::class);
C::isThis(null);
C::isThis(true);
C::isThis(1.5);
C::isThis(1);
C::isThis(STDIN);

echo "\n";

D::isThis(C::class);
D::isThis(D::class);
D::isThis(stdClass::class);
D::isThis(null);
D::isThis(true);
D::isThis(1.5);
D::isThis(1);
D::isThis(STDIN);

echo "\n";

T::isThis(C::class);
T::isThis(D::class);
T::isThis(T::class);
