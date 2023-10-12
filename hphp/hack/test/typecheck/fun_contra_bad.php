<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function TakeFun((function(arraykey): arraykey) $f): void {
  ($f)(3);
}
function Pass((function(int): int) $g): void {
  TakeFun($g);
}
function DoIt(): void {
  Pass((int $x) ==> 5);
}

function main(): void {
  DoIt();
}
