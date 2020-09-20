<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

enum Pos: int {}

function f(Pos $x): void {
  hh_show($x);
}
