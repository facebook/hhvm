<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function expectFloat(float $x): void {}
function expectInt(int $x): void {}
function expectNum(num $x): void {}

function testfloat(num $n):void {
  $m = Vector { 3.4 };
  $x = $m[0] + $n;
  expectFloat($x);
  $m = Vector { 3.4 };
  $x = $m[0] - $n;
  expectFloat($x);
  $m = Vector { 3.4 };
  $x = $m[0] * $n;
  expectFloat($x);
  $m = Vector { 3.4 };
  $x = $m[0] / $n;
  expectFloat($x);
  $m = Vector { 3.4 };
  $x = $m[0] + $m[0];
  expectFloat($x);
  $m = Vector { 3.4 };
  $x = $m[0] - $m[0];
  expectFloat($x);
  $m = Vector { 3.4 };
  $x = $m[0] * $m[0];
  expectFloat($x);
  $m = Vector { 3.4 };
  $x = $m[0] / $m[0];
  expectFloat($x);
  $m = Vector { 3.4 };
  $x = -$m[0];
  expectFloat($x);
}

/*
function testfloat2(bool $b, num $n):void {
  $m = $b ? Vector { } : varray[3.4];
  $x = $m[0] + $n;
  expectFloat($x);
  $m = $b ? Vector { } : Vector { 3.4 };
  $x = $m[0] - $n;
  expectFloat($x);
  $m = $b ? Vector { } : Vector { 3.4 };
  $x = $m[0] * $n;
  expectFloat($x);
  $m = $b ? Vector { } : Vector { 3.4 };
  $x = $m[0] / $n;
  expectFloat($x);
  $m = $b ? Vector { } : Vector { 3.4 };
  $x = $m[0] + $m[0];
  expectFloat($x);
  $m = $b ? Vector { } : Vector { 3.4 };
  $x = $m[0] - $m[0];
  expectFloat($x);
  $m = $b ? Vector { } : Vector { 3.4 };
  $x = $m[0] * $m[0];
  expectFloat($x);
  $m = $b ? Vector { } : Vector { 3.4 };
  $x = $m[0] / $m[0];
  expectFloat($x);
  $m = $b ? Vector { } : Vector { 3.4 };
  $x = -$m[0];
  expectFloat($x);
}
*/

function testints():void {
  $m = Vector { 3 };
  $x = $m[0] + $m[0];
  expectInt($x);
  $m = Vector { 3 };
  $x = $m[0] - $m[0];
  expectInt($x);
  $m = Vector { 3 };
  $x = $m[0] * $m[0];
  expectInt($x);
  $m = Vector { 3 };
  $x = -$m[0];
  expectInt($x);
}

function testint(): void {
  $m = Vector {};
  expectNum($m[0] + 1);
  $m[] = 0.1; // ok, but can't infer
  $m[] = 1; // ok

  $m = Vector {};
  expectFloat($m[0] + 1);
  $m[] = 0.1; // ok, but can't infer
  $m[] = 1; // nok

  $m = Vector {};
  expectInt($m[0] + 1);
  $m[] = 1; // ok, but can't infer
  $m[] = 0.1; // nok
}
