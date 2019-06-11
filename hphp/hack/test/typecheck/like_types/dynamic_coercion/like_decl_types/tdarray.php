<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

// darrays are only enforced as arrays, so it is not safe to allow dynamic to coerce to darray
function without_targs(~darray $a): darray {
  return $a; // error
}

function with_targs(~darray<int, string> $a): darray<int, string> {
  return $a; // error
}
