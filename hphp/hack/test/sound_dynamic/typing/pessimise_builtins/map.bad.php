<?hh

class C {}

function test_map(mixed $m, Map<int,C> $v) : Map<int,C> {
  $v[0] = $m;
  return Map<int,dynamic>{};
}
