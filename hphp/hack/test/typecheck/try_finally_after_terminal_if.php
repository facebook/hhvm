<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function takes_int(int $x): void {}

function test(?string $x): void {
  if ((int)$x === 0) {
    return;
  }
  $x = (int)$x;
  try {
  } finally {
    takes_int($x);
  }
}
