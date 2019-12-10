<?hh // partial
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function expect_darray(darray $_): void {}

function with_omitted_generics(): void {
  expect_darray(darray['a' => 1, 'b' => 2]);
}

function with_default_and_anonymous_variadic(float $x, ?string $y = null, ...): void {}

function call_with_default_and_anonymous_variadic(string $s): void {
  with_default_and_anonymous_variadic(3.14);
  with_default_and_anonymous_variadic(3.14, 'pi');
  with_default_and_anonymous_variadic(3.14, '%s', $s);
}

newtype N as arraykey = int;
