<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test(shape('x' => int, ?'y' => nothing) $s): shape('x' => int) {
  return $s;
}
