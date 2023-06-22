<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

function expect_arraykey(arraykey $_): void { }

function main(vec<arraykey> $v): void {
  $t = $v as (int, mixed);
  list($a, $b) = $t;
  expect_arraykey($b);
}
