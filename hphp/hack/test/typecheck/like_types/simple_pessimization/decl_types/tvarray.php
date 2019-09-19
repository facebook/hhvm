<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

function without_targs(varray $x): void {
  hh_show($x);
}

function with_targs(varray<int> $x): void {
  hh_show($x);
}
