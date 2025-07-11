<?hh
function f(?vec<int> $v): (int | string) {
  $x = $v[0] ?? "a";
  hh_expect_equivalent<(int | string)>($x);
  return $x;
}
