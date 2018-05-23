<?hh //strict

function f(array<int> $x): void {
  // this should be an error, since it can never be true
  if ($x === null) {
  }
}
