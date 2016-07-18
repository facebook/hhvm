<?hh

function foo(keyset<int> $x): keyset<int> {
  $x[] = 42;
  return $x;
}
