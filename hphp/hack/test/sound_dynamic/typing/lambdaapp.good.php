<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__SupportDynamicType>>
function test(
  vec<string> $value,
  supportdyn<(function(vec<string>):~int)> $f,
): void {
  $g = (vec<string> $x) ==> 5;
  $g($value);
  $f($value);
}
