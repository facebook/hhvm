<?hh

class C {}

function test_vec(mixed $m, Vector<C> $v) : Vector<C> {
  $v[] = $m;
  $v[0] = $m;
  return Vector<dynamic>{};
}
