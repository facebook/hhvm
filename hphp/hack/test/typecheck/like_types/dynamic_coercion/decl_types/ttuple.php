<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function dyn(): dynamic { return varray[]; }

function f(): (int, int) {
  return dyn(); // error
}
