<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__DynamicallyCallable>>
function foo(vec<int> $x):vec<num> {
  return vec[$x[0]];
}

<<__DynamicallyCallable>>
function testit():num {
  $x = HH\FIXME\UNSAFE_CAST<vec<string>,vec<int>>(vec["A"]);
  // Should have type ~vec<num>
  $y = foo($x);
  return $y[0];
}
