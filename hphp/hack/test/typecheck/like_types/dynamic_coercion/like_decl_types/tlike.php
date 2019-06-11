<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

// this is just basic subtyping

function f(~int $a): ~int {
  return $a; // ok
}

function g(~arraykey $a): ~int {
  return $a; // error
}
