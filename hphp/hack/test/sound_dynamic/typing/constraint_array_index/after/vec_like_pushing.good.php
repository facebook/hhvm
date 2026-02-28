<?hh
function test1(~vec<int> $v):void {
  $g = $v[0];
  hh_expect_equivalent<~int>($g);
}
