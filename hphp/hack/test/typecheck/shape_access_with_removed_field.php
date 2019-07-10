<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function test(shape('f' => int, ...) $s): void {
  Shapes::removeKey(inout $s, 'f');
  $s['f'] ?? null;
}
