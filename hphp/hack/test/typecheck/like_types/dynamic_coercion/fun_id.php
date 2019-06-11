<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function f(int $i): void {}

function test(dynamic $d): void {
  fun('f')($d);
}
