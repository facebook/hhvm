<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function omg(): (?string, array<int>) {
  $x = array();
  return $x;
}

function breakit(): void {
  $x = omg();
  list($y, $z) = $x;
  echo $y;
}

//breakit();
