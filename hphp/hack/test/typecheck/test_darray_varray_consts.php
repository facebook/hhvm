<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  const darray<string, string> FOO = dict['foo' => 'bar'];
  const varray<varray<string>> BAR = vec[vec['foo']];
  const varray<dict<string, vec<string>>>
    BAZ = vec[dict['foo' => vec['bar']]];

  // Should not typecheck
  public function test(): string {
    return self::FOO;
  }
}
