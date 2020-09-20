<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function f(~mixed $a): mixed {
  return $a; // ok
}
