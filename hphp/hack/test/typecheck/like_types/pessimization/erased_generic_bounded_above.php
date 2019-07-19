<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function expect_arraykey(arraykey $x): void {}

function f<T as arraykey>(T $t): void {
  expect_arraykey($t); // T as ~arraykey
}
function g<T as int>(T $t): void {
  expect_arraykey($t); // T as ~int
}
function h<T as bool>(T $t): void {
  expect_arraykey($t); // T as ~bool
}
