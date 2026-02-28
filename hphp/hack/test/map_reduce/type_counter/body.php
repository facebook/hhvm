<?hh

function f(vec<int> $v): void {
  $i = $v[0]; // One ~int on RHS, one on LHS, one on the whole expression
  $v; // No like type here
  $i; // One ~int here
}
