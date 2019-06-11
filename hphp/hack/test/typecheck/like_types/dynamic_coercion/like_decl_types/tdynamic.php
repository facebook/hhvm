<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function f(~int $d): dynamic {
  return $d; // ok
}
