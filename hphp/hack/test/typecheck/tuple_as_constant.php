<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  const (int, string, vec<string>) MY_TUPLE = tuple(0, "foo", vec["b", "bc"]);

  public function return_int(): int {
    return self::MY_TUPLE[0];
  }

  public function return_string(): string {
    return self::MY_TUPLE[1];
  }

  public function return_wrong(): dict<string, int> {
    return self::MY_TUPLE[2];
  }
}
