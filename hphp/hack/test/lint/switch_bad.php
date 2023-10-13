<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test1(float $x): void {
  switch ($x) {
    case 'foo': return;
  }
}

class A {}
class B {}

function test2(A $x): void {
  switch ($x) {
    case new B(): return;
  }
}
