<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

type TS = shape("a" => ?string);

function expectString(string $_):void { }

function testit(TS $s):void {
  $x = $s['a'] ?? 'blah';
  expectString($x);
}
