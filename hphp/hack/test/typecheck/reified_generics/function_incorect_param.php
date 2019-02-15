<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function f<reify Tu, Tv>(Tu $i, Tv $j): void {}
function g<Tu, Tv>(Tu $i, Tv $j): void {}

function h(): void {
  f<int, string>(3, "4");
  g<int, string>(3, "4");
}
