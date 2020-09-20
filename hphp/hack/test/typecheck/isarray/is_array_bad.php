<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

function test_is_array(array $m): ?varray_or_darray<int> {
  if (HH\is_php_array($m)) {
    return $m;
  }
  return null;
}

function test1(darray<int, string> $x): darray<int, int> {
  invariant(HH\is_php_array($x), '');
  return $x; // invalid return type
}

function test2(darray<int, string> $x): varray<string> {
  invariant(HH\is_php_array($x), '');
  return $x; // invalid return type, expect varray, not darray
}

function test3(array $x): varray<string> {
  invariant(HH\is_php_array($x), '');
  return $x; // invalid return type, expected varray, not varray_or_darray
}

function test4(array $x): darray<int, string> {
  invariant(HH\is_php_array($x), '');
  return $x; // invalid return type, expected varray, not varray_or_darray
}
