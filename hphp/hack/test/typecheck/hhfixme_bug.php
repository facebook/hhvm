<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function testit(
  KeyedTraversable<arraykey, mixed> $kt
  ): void {
    $m = new Map($kt);
}
