<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__SupportDynamicType>>
function genericfun<T as supportdyn<mixed> >(
  ?supportdyn<(function(vec<T>): void)> $f,
): ~Vector<T> {
  throw new Exception("A");
}

<<__SupportDynamicType>>
function testit(): void {
  $f = genericfun(($ts): void ==> { $x = $ts[3]; });
}
