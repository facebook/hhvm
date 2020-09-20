<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

function dyn(): dynamic { return varray[4, 5, 6]; }

// not all HHVM arrays are considered varray_or_darray,
// so dynamic ~> varray_or_darray is invalid
function without_targs(): varray_or_darray {
  return dyn(); // error
}

function with_targs(): varray_or_darray<int> {
  return dyn(); // error
}
