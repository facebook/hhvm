<?hh

<<__SupportDynamicType>>class C {}

function test_map(mixed $m, Map<int,C> $v, ~int $i) : Map<int,C> {
  $v[0] = $m;
  $v[0] = 1;
  $v[0] = $i;
  Map<int, C>{1=>$i};
  return Map<int,dynamic>{};
}
