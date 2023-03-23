<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

function getStrings():~vec<string> {
  return vec[];
}
function getArraykeys():~vec<arraykey> {
  return vec[];
}
function test1(string $s, dynamic $d, arraykey $ak):void {
  $ls = getStrings()[0];
  $lak = getArraykeys()[0];

  // Expect Pair<string,string> and Pair<string,arraykey>
  $a = Pair<string,string>{$s, $s};
  hh_expect_equivalent<Pair<string,string>>($a);
  $b = Pair<string,arraykey>{$s, $s};
  hh_expect_equivalent<Pair<string,arraykey>>($b);

  // Like types should be accepted as elements
  $c = Pair<string,string>{$ls,$s};
  hh_expect_equivalent<Pair<~string,string>>($c);

  // But a supertype should be respected as a type argument
  $d = Pair<string, arraykey>{$ls,$ls};
  hh_expect_equivalent<Pair<~string,~arraykey>>($d);

  // Obvs we should be allowed to ask for a like type
  $e = Pair<~string,~arraykey>{$s,$s};
  hh_expect_equivalent<Pair<~string,~arraykey>>($e);
}
