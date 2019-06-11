<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

// The runtime does not enforce the type returned in a yield from
function yf(dynamic $d): string {
  yield from $d;
}

function test(): void {
  foreach(yf(varray[3,4]) as $val) {}
}
