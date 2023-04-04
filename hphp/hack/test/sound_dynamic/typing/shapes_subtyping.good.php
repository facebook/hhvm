<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

type TS1 = supportdyn<shape(
    'foo' => int,
    ...
)>;

type TS2 = supportdyn<shape(
  'foo' => int,
  ?'bar' => mixed,
  ...
)>;

type TS3 = supportdyn<shape(
  'foo' => int,
  ?'bar' => supportdyn<mixed>,
  ...
)>;

type TS4 = shape('foo' => int, ?'bar' => supportdyn<mixed>);
type TS5 = supportdyn<shape('foo' => int, 'bar' => mixed)>;

function expect1(TS1 $_):void { }
function expect2(TS2 $_):void { }
function expect3(TS3 $_):void { }
function expect4(TS4 $_):void { }

function testit(TS1 $x, TS2 $y, TS3 $z, TS4 $w, TS5 $v):void {
  expect1($x); expect1($y); expect1($z); expect1($w);
  expect2($x); expect2($y); expect2($z); expect2($w);
  expect3($x); expect3($y); expect3($z); expect3($w);
  expect4($v);
}
