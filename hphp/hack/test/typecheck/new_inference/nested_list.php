<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function make_it(): (vec<int>, vec<string>) {
  return tuple(vec[2,3], vec["a", "b"]);
}

function consume_it(): (int, int, string, string) {
  list(list($a, $b), list($c, $d)) = make_it();
  return tuple($a, $b, $c, $d);
}
