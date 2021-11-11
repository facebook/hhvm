<?hh

<<__SupportDynamicType>>class C {}

function test_vec(mixed $m, Vector<C> $v, vec<~int> $vi) : Vector<C> {
  $i = $vi[0];
  $v[0] = $m;
  $v[0] = 1;
  $v[0] = $i;
  $v[] = $m;
  $v[] = 1;
  $v[] = $i;
  Vector<C>{$i};
  return Vector<dynamic>{};
}
