<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function f(): ?shape('x' => ?int) {
  return null;
}

function test(): int {
  return Shapes::idx(f() ?? shape(), 'x', 0);
}
