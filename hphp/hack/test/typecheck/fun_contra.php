<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function TakeFun((function(int): arraykey) $f): void {
  ($f)(3);
}
function Pass((function(arraykey): int) $g): void {
  TakeFun($g);
}
function DoIt(): void {
  Pass((arraykey $x) ==> 5);
}

function main(): void {
  DoIt();
}
