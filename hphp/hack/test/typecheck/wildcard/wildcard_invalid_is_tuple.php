<?hh

function f(mixed $x): void {
  $x is (_);
  $x is (_, int);
  $x is (int, _);
  $x is (_, _);
  $x is ((_), (int));
  $x is ((int, _), (_));
  $x is (int, (_, int));
  $x is (_, (int, _));
}
