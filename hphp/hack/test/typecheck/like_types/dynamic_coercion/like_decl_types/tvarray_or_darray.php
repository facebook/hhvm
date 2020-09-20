<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

function without_targs(~varray_or_darray $a): varray_or_darray {
  return $a; // not an error
}

function with_targs(~varray_or_darray<int> $a): varray_or_darray<int> {
  return $a; // error
}
