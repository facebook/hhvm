<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function f<T as int>(): void {}

function test(): void {
  f<dynamic>();
  f<int>(); // implicitly ~int
  f<~int>();
  f<string>();
}
