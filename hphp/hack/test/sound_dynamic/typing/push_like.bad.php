<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// No SDT
class C {}

function get():~int {
  return 3;
}

function return_pair_direct():~(int,C) {
  return tuple(get(), new C());
}

function return_pair():~(int,C) {
  $x = tuple(get(), new C());
  return $x;
}
