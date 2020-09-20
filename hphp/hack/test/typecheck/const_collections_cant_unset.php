<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  const array<string, string> FOO = darray['foo' => 'bar'];

  public function modify_const(): void {
    unset(self::FOO['foo']);
  }
}
