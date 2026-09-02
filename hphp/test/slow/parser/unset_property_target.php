<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public int $p = 1;
  public static int $sp = 2;
}

<<__EntryPoint>>
function main(): void {
  $a = new A();
  unset($a->p);
}
