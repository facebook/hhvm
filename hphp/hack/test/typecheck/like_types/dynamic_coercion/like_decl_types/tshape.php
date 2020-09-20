<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function f(~shape('a' => int) $s): shape('a' => int) {
  return $s; // error
}
