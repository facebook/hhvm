<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function __construct(public int $item) { }
}
function testit(?Map<string, C> $m):int {
  $m = $m ?? Map {};
  $y = $m["a"];
  $x = $y->item;
  return $x;
}
