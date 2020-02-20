<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

function dyn(): dynamic { return varray[3, 4]; }

// darrays are only enforced as arrays, so it is not safe to allow dynamic to
// coerce to darray

function without_targs(): darray {
  return dyn(); // error
}

function with_targs(): darray<int, string> {
  return dyn(); // error
}
