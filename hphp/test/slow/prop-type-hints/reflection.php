<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public static int $x = 123;
}

class B extends A {}
<<__EntryPoint>> function main(): void {
hphp_set_static_property('B', 'x', 'abc', false);
}
