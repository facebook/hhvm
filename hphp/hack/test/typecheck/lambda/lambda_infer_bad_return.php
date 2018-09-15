<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function foo(): (function(int): string) {
  return $x ==> true;
}
