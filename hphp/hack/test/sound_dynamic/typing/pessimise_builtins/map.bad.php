<?hh

<<__SupportDynamicType>>class C {}

function test_map(mixed $m, Map<int,C> $v, vec<~int> $vi) : Map<int,C> {
  $i = $vi[0];
  $v[0] = $m;
  $v[0] = 1;
  $v[0] = $i;
  Map<int, C>{1=>$i};
  return Map<int,dynamic>{};
}
