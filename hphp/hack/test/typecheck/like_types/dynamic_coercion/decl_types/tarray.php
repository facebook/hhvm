<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

function dyn(): dynamic { return 4; }

function akvec(): varray<int> {
  return dyn(); // error
}

function akmap(): darray<int, string> {
  return dyn(); // error
}
