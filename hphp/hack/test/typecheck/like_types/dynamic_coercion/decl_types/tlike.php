<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

// this is just basic subtyping

function dyn(): dynamic { return 4; }

function f(): ~int {
  return dyn(); // ok
}
