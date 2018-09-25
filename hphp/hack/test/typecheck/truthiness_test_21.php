<?hh // strict

type A = shape(?'a' => int);

function test(A $x): void {
  if ($x) {
  }
}
