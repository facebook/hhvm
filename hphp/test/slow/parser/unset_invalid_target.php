<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public int $p = 1;
}

<<__EntryPoint>>
function main(): void {
  $x = 1;
  unset($x);
}
