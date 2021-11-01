<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function dyn(): dynamic { return 4; }

function f() /* : TAny */{
  return dyn(); // ok
}
