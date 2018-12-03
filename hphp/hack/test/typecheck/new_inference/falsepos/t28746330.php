<?hh // strict

function f<T>(T $_, T $_): void {}

function test(mixed $x): arraykey {
  if (false) {
    $y = 42;
  } else {
    $y = 'foo';
  }
// $y: int|string
// Generate fresh v for T
//   mixed <: v
//   int|string <: v
// => int <: v /\ string <: v
  f($x, $y);
  return $y;
}
