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
  $a = dict<string,int>['a' => $i];
  hh_expect_equivalent<dict<string,int>>($a);
  $b = dict<string,int>['a' => $i, 'b' => $li];
  hh_expect_equivalent<dict<string,~int>>($b);
  $c = dict<string,int>['a' => $i, 'b' => $dyn];
  hh_expect_equivalent<dict<string,~int>>($c);
  $d = dict<string,arraykey>['a' => $i, 'b' => $li];
  hh_expect_equivalent<dict<string,~arraykey>>($d);
  $d[$ls] = 5;
  hh_expect_equivalent<dict<(~string & arraykey),~arraykey>>($d);
  $e = dict<string,int>[$ls => $i];
  hh_expect_equivalent<dict<(~string & arraykey),int>>($e);
  $f = dict<string, dict<string, int>>['bar' => dict['baz' => 42]];
  expectInt($f['a']['b']);
}
