<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

// not all HHVM arrays are considered varray_or_darray,
// so dynamic ~> varray_or_darray is invalid
function without_targs(~varray_or_darray $a): varray_or_darray {
  return $a; // error
}

function with_targs(~varray_or_darray<int> $a): varray_or_darray<int> {
  return $a; // error
}
