<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public static int $sp = 2;
}

<<__EntryPoint>>
function main(): void {
  unset(A::$sp);
}
