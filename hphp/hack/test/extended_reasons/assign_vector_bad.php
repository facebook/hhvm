<?hh

function test(Vector<string> $v): Vector<arraykey> {
  $v[0] = 42;
  return $v;
}
