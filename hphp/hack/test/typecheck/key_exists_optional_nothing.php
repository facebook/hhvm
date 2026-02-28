<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test(shape(?'a' => nothing) $s): void {
  if (Shapes::keyExists($s, 'a')) {
  }
}
