<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

function my_idx<Tv>(shape(?'a' => Tv) $_):?Tv {
  return null;
}

function testit(shape(?'a' => ?int) $s):void {
  $x = my_idx($s);
}
