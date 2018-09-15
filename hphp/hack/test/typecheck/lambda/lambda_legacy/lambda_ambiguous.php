<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function testit(): void {
  $f = $x ==> $x + 1;
  $f(3);
}
