<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function g<T1 as string>(?T1 $x): void {}

function test1<T2 as ?string>(T2 $x): void {
  g($x); // #0 < string && (T2 <: ?#0 || ?string <: ?#0)
}

function test2<T2 as ?int>(T2 $x): void {
  g($x);
}
