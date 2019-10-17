<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function f<reify T>(): void {}

function main(): void {
  f<vec<int>>();
  f<keyset<int>>();
  f<dict<int, int>>();
}
