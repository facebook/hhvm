<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  const array<string> FOO = varray['bar', 'foo'];

  public function modify_const(): void {
    self::FOO[0] = 'invalid';
  }
}
