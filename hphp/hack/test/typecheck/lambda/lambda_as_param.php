<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function expectIntToAK((function(string, int, int): arraykey) $f): void {
}

function expectIntToInt((function(string, int, int): int) $f): void {
}
function expectStringToInt((function(string, string, int): int) $f): void {
}
function testLambdaAsParam(): void {
  $f = (string $s, $i, $j) ==> 3;
  expectIntToAK($f);
  expectIntToInt($f);
  expectStringToInt($f);
  // Contextual case
  expectIntToInt(($s, $x, $y) ==> 5);
}
