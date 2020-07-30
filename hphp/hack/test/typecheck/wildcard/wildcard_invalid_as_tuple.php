<?hh

function f(mixed $x): void {
  $x as (_);
  $x as (_, int);
  $x as (int, _);
  $x as (_, _);
  $x as ((_), (int));
  $x as ((int), (_));
  $x as ((int, _), (_));
  $x as (int, (_, int));
  $x as (_, (int, int));
  $x as (_, (int, _));
}
