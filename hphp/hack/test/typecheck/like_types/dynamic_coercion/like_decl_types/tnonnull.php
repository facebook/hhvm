<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function f(~nonnull $a): nonnull {
  return $a; // ok
}
