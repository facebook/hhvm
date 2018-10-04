<?hh // strict

// Copyright 2004-present Facebook. All Rights Reserved.
function f(inout string $_): void {}

function test_good(vec<string> $v): void {
  f(inout $v[0]);
}

function test_bad_1<T as vec<string>>(T $v): void {
  f(inout $v[0]);
}

function test_bad_2<T>(T $v): void where T = vec<string> {
  f(inout $v[0]);
}
