<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function f(~(int, int) $s): (int, int) {
  return $s; // error
}
