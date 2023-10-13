<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  const darray<string, string> FOO = darray['foo' => 'bar'];
  const varray<varray<string>> BAR = varray[varray['foo']];
  const varray<dict<string, vec<string>>>
    BAZ = varray[dict['foo' => vec['bar']]];

  // Should not typecheck
  public function test(): string {
    return self::FOO;
  }
}
