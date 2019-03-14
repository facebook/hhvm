<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

interface I {}

// Weirdly, if B is undefined, we used to get no error in foo
class C extends B {}

function foo(C $x): I {
  return $x;
}
