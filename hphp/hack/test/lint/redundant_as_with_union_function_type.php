<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

function redundant_as_with_union_function_type(bool $b, mixed $x): void {
  // create a union of function types that does not get simplified
  $f = $b ? (int $_): int ==> 0 : (arraykey $_): arraykey ==> 0;
  $f($x as int); // should be no warning
  $x as int; // ensure we still get an warning here
}
