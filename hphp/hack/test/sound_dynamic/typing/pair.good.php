<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

function getLikeInt():~vec<int> { $x = vec["A" upcast dynamic]; return $x; }
function make():void {
  $p = Pair{getLikeInt()[0],"A"};
  hh_expect_equivalent<Pair<~int,string>>($p);
  $x = $p[0];
  hh_expect_equivalent<~int>($x);
}
