<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

function without_targs(varray_or_darray $x): void {
  hh_show($x);
}

function with_targs(varray_or_darray<int> $x): void {
  hh_show($x);
}
