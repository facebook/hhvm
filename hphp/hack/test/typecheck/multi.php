<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

function expectTwo(int $x, bool $y):void { }
function expectPair((int,bool) $p):void { }
function expectShape(shape('a' => int, 'b' => bool, 'c' => string) $s):void { }

function testThem():void {
  expectTwo("A", 2.3);
  expectPair(tuple("A", 2.3));
  expectShape(shape('a' => "A", 'b' => 2.3));
  $s = shape('a' => "C", 'b' => 4.5);
  expectShape($s);
}
