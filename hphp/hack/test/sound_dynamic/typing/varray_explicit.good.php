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
  $a = varray<int>[$i];
  hh_expect_equivalent<varray<int>>($a);
  $b = varray<int>[$i, $li];
  hh_expect_equivalent<varray<~int>>($b);
  $c = varray<int>[$i, $d];
  hh_expect_equivalent<varray<~int>>($c);
  $d = varray<arraykey>[$i, $li];
  hh_expect_equivalent<varray<~arraykey>>($d);
  $e = varray<~int>[$i];
  hh_expect_equivalent<varray<~int>>($e);
  $f = varray<varray<int>>[varray[2,3]];
  expectInt($f[0][1]);
}
