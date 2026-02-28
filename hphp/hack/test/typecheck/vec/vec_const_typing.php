<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  const vec<string> MY_VEC = vec["foo", "bar", "baz"];

  public function return_string(): string {
    return self::MY_VEC[0];
  }

  public function return_int(): int {
    return self::MY_VEC[0];
  }
}
