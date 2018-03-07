<?hh

final class C {}
interface I {};

function is_mixed(mixed $x): void {
  if ($x is classname<mixed>) {
    echo "classname\n";
  } else {
    echo "not classname\n";
  }
}

function is_nonnull(mixed $x): void {
  if ($x is \classname<nonnull>) {
    echo "classname\n";
  } else {
    echo "not classname\n";
  }
}

function is_dynamic(mixed $x): void {
  if ($x is classname<dynamic>) {
    echo "classname\n";
  } else {
    echo "not classname\n";
  }
}

is_mixed(C::class);
is_mixed(I::class);
is_mixed('C');
is_mixed('I');
is_mixed(stdClass::class);
is_mixed(new C());
is_mixed(true);
is_mixed(1);
is_mixed(1.5);
is_mixed(STDIN);

echo "\n";

is_nonnull(C::class);
is_nonnull(I::class);
is_nonnull('C');
is_nonnull('I');
is_nonnull(stdClass::class);
is_nonnull(new C());
is_nonnull(true);
is_nonnull(1);
is_nonnull(1.5);
is_nonnull(STDIN);

echo "\n";

is_dynamic(C::class);
is_dynamic(I::class);
is_dynamic('C');
is_dynamic('I');
is_dynamic(stdClass::class);
is_dynamic(new C());
is_dynamic(true);
is_dynamic(1);
is_dynamic(1.5);
is_dynamic(STDIN);
