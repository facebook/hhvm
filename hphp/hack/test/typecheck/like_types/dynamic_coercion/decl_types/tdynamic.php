<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function dyn(): dynamic { return 4; }

function f(): dynamic {
  return dyn(); // ok
}
