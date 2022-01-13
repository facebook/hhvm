<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function foo(): int {
  return 1;
}

function get_arr(): vec<int> {
  return vec[];
}

function bar(): void {
  $v = vec[0];

  // Ban array indexing with a function receiver
  // in an lvalue position.

  // Good: array update in a local variable.
  $v[foo()] = 1;
  $v[get_arr()[0]] = 12;

  // Bad: array update in a temporary value.
  get_arr()[] = 1;
  get_arr()[0] = 1;
  list($_, get_arr()[0]) = vec[1, 2];

  $vals = vec[1, 2];
  foreach($vals as get_arr()[0]) {}
}
