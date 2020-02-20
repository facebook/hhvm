<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

function dyn(): dynamic { return darray["3" => 4]; }

// varrays are only enforced as arrays, so it is not safe to allow dynamic to
// coerce to varray

function without_targs(): varray {
  return dyn(); // error
}

function with_targs(): varray<int> {
  return dyn(); // error
}
