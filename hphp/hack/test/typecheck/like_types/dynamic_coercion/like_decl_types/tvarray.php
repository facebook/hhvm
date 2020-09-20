<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

// varrays are only enforced as arrays, so it is not safe to allow dynamic to coerce to varray
function without_targs(~varray $a): varray {
  return $a; // error
}

function with_targs(~varray<int> $a): varray<int> {
  return $a; // error
}
