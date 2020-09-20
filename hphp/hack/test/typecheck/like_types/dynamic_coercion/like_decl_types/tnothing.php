<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function f(~nothing $a): nothing {
  return $a; // ok
}
