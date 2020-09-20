<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function dyn(): dynamic { return 4; }

enum Pos: int {}

function f(): Pos {
  return dyn(); // error
}
