<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test(shape('x' => vec<int>) $s): shape('x' => vec<num>) {
  $s['x'][] = 3.14;
  return $s;
}
