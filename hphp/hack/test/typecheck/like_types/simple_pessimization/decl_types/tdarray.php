<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

function without_targs(darray $x): void {
  hh_show($x);
}

function with_targs(darray<int, string> $x): void {
  hh_show($x);
}
