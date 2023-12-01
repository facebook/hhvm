<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  const darray<string, string> FOO = dict['foo' => 'bar'];

  public function modify_const(): void {
    unset(self::FOO['foo']);
  }
}
