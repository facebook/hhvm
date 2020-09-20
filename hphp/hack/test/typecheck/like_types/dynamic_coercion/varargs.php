<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

// The runtime doesn't enforce the type of varargs
function f(int ...$i): void {}

function d(dynamic $d): void {
  f($d);
}

<<__EntryPoint>>
function main(): void {
  f("str");
}
