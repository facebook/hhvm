<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

function getStrings():~vec<string> {
  return vec[];
}
function getArraykeys():~vec<arraykey> {
  return vec[];
}
function test1(string $s, dynamic $dyn, arraykey $ak):void {
  $ls = getStrings()[0];
  $lak = getArraykeys()[0];

  $a = keyset<string>[$s];
  hh_expect_equivalent<keyset<string>>($a);
  // Like types should be accepted as elements
  $c = keyset<string>[$ls,$s];
  hh_expect_equivalent<keyset<(~string & arraykey)>>($c);

  $d = keyset<string>[$dyn];
  hh_expect_equivalent<keyset<(~string & arraykey)>>($c);

  // But a supertype should be respected as a type argument
  $d = keyset<arraykey>[$ls,$s];
  hh_expect_equivalent<keyset<arraykey>>($d);
}
