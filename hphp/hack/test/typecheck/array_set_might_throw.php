<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class Foo { }

<<__EntryPoint>>
function test(): void {
  $x = new Foo();
  try {
    $y = vec[];
    $y[4] = null; // hack assumes this never throws
    $x = 1; // so hack thinks this always occurs
  } catch (Exception $e) {
  }

  $x += 5; // boom
}
