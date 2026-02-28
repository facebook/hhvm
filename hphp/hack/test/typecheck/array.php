<?hh

function f(varray<int> $x): void {
  // this should be an error, since it can never be true
  if ($x === null) {
  }
}
