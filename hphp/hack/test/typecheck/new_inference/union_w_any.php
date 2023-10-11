<?hh

function test(
  vec<_> $v1,
  vec<int> $v2,
  bool $b,
): void {
    hh_show($b ? $v1 : $v2);
    hh_show($b ? $v2 : $v1);
}
