<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

function foo(C<int> $x): C {
  return $x;
}

function bar(C $x): C<int> {
  return $x;
}
