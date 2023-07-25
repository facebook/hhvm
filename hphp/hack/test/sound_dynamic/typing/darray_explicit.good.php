<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

function getVec():~vec<int> {
  return vec[];
}

function getStrings():~vec<string> {
  return vec[];
}

<<__NoAutoDynamic>>
function expectInt(int $_):void { }

function test1(dynamic $dyn, int $i):void {
  $li = getVec()[0];
  $ls = getStrings()[0];
  $a = darray<string,int>['a' => $i];
  hh_expect_equivalent<darray<string,int>>($a);
  $b = darray<string,int>['a' => $i, 'b' => $li];
  hh_expect_equivalent<darray<string,~int>>($b);
  $c = darray<string,int>['a' => $i, 'b' => $dyn];
  hh_expect_equivalent<darray<string,~int>>($c);
  $d = darray<string,arraykey>['a' => $i, 'b' => $li];
  hh_expect_equivalent<darray<string,~arraykey>>($d);
  $d[$ls] = 5;
  hh_expect_equivalent<darray<(~string & arraykey),~arraykey>>($d);
  $e = darray<string,int>[$ls => $i];
  hh_expect_equivalent<darray<(~string & arraykey),int>>($e);
  $f = darray<string, darray<string, int>>['bar' => darray['baz' => 42]];
  expectInt($f['a']['b']);
}
