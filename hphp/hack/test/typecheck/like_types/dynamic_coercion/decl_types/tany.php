<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

function dyn(): dynamic { return 4; }

function f() {
  return dyn(); // ok
}
