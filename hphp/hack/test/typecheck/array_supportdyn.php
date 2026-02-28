<?hh

function get<T>(Vector<T> $v): T {
  return $v[0];
}

function expect_dict(dict<arraykey, mixed> $d): void {
}

<<__NoAutoDynamic>>
function test(supportdyn<KeyedContainer<arraykey, mixed>> $kc): void {
  $v = Vector{$kc};
  $kc2 = get($v);
  $m = $kc2['a'];
  $d = $m as dict<_, _>;
  expect_dict($d);
}
