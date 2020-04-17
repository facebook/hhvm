<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function make_int():int {
  return 3;
}

function bar<T>(T $x): void {
}

function foo():void {
  bar(make_int());
  bar("a");
}
