<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

function akvec(~varray<int> $a): varray<int> {
  return $a; // error
}

function akmap(~darray<int, string> $a): darray<int, string> {
  return $a; // error
}
