<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

function test1(darray<int, string> $x): darray<int, int> {
  invariant(HH\is_php_array($x), '');
  return $x; // invalid return type
}

function test2(darray<int, string> $x): varray<string> {
  invariant(HH\is_php_array($x), '');
  return $x; // invalid return type, expect varray, not darray
}
