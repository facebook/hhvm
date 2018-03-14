<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function make_it(): (function(int): mixed) {
  return async (int $x) ==> $x + 1;
}
