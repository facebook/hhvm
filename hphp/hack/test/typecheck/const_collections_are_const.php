<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  const varray<string> FOO = vec['bar', 'foo'];

  public function modify_const(): void {
    self::FOO[0] = 'invalid';
  }
}
