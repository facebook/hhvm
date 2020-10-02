<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

function akany(varray_or_darray $x): void {
  hh_show($x);
}

function akvec(varray<int> $x): void {
  hh_show($x);
}

function akmap(darray<int, string> $x): void {
  hh_show($x);
}
