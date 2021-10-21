<?hh

<<__SupportDynamicType>>class C {}

function test_vec(mixed $m, Vector<C> $v, ~int $i) : Vector<C> {
  $v[0] = $m;
  $v[0] = 1;
  $v[0] = $i;
  $v[] = $m;
  $v[] = 1;
  $v[] = $i;
  Vector<C>{$i};
  return Vector<dynamic>{};
}
