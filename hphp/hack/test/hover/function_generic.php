<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

function identity<T>(T $x, mixed $_):T {
  return $x;
}

function testit():void {
  $x = identity(3, "A");
  //   ^ hover-at-caret
}
