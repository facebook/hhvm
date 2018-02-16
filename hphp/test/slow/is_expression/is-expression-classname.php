<?hh // strict

abstract class C {}
final class D extends C {}
final class E {}

function is_C(mixed $x): void {
  if ($x is classname<C>) {
    echo "classname\n";
  } else {
    echo "not classname\n";
  }
}

is_C(C::class);
is_C(D::class);
is_C(new D());
is_C(E::class);
is_C(new E());
is_C(true);
is_C(1.5);
