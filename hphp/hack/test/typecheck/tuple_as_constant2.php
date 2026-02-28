<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function get_int(): int {
  return 0;
}

class A {
  const (int, int) MY_TUPLE = tuple(get_int(), get_int());
}
