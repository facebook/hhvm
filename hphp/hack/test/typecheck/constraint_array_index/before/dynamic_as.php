<?hh
function f(dynamic $d): void {
  $data = $d as dict<_, _>;
  $v = $data[0];
  hh_expect_equivalent<(dynamic & mixed)>($v);
}
