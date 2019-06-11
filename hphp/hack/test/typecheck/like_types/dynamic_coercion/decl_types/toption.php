<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function dyn(): dynamic { return varray[]; }

function enforceable(): ?int {
  return dyn(); // ok
}

function unenforceable(): ?(int, int) {
  return dyn(); // error
}
