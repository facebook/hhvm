<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

function expect_dynamic(dynamic $_):void { }

function test(dynamic $d):void {
  $s = type_structure($d, 'TC')['fields'];
  expect_dynamic($s);
}
