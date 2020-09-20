<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function f<reify Tu>(Tu $i): void {}
function g(int $j): void {}

function h(): void {
  f(3);
  g<int>(4);
}
