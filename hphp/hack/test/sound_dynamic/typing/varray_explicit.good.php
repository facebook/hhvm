<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

function getVarray():~varray<int> {
  return vec[];
}

function ret():varray<~int> {
  return vec[2];
}

<<__NoAutoDynamic>>
function expectInt(int $_):void { }

function test1(dynamic $d, int $i):void {
  $li = getVarray()[0];
  $a = vec<int>[$i];
  hh_expect_equivalent<vec<int>>($a);
  $b = vec<int>[$i, $li];
  hh_expect_equivalent<vec<~int>>($b);
  $c = vec<int>[$i, $d];
  hh_expect_equivalent<vec<~int>>($c);
  $d = vec<arraykey>[$i, $li];
  hh_expect_equivalent<vec<~arraykey>>($d);
  $e = vec<~int>[$i];
  hh_expect_equivalent<vec<~int>>($e);
  $f = vec<varray<int>>[vec[2,3]];
  expectInt($f[0][1]);
}
