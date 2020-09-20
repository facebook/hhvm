<?hh

function f(mixed $x): void {
  $x is (int, vec<_>);
  $x is (int, (vec<_>, string));
  $x is (int, dict<_, _>);
  $x is (vec<_>, int);
  $x is ((vec<_>, string), int);
  $x is (dict<_, _>, int);
  $x as (int, vec<_>);
  $x as (int, (vec<_>, string));
  $x as (int, dict<_, _>);
  $x as (vec<_>, int);
  $x as ((vec<_>, string), int);
  $x as (dict<_, _>, int);
}
