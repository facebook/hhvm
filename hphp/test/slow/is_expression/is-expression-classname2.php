<?hh

final class C {}
interface I {};

function is_classname(mixed $x): void {
  if ($x is \classname) {
    echo "classname\n";
  } else {
    echo "not classname\n";
  }
}

is_classname(C::class);
is_classname(I::class);
is_classname('C');
is_classname('I');
is_classname(new C());
is_classname(true);
is_classname(1);
is_classname(1.5);
is_classname(STDIN);
