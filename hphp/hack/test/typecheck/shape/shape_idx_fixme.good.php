<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

function test(shape('a' => int, ...) $s, string $y):void {
  /* HH_FIXME[4051] */
  $x = Shapes::idx($s, $y);
}
