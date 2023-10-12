<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function doh(bool $b):void {
  if ($b) {
    $f = ($x) ==> $x;
  } else {
    $f = (int $x, string $y) ==> $y;
  }
}
