<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function test(shape() $s): void {
  $s['f'] ?? null;
}
