<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function dyn(): dynamic { return darray[]; }

function f(): shape('a' => int) {
  return dyn(); // error
}
