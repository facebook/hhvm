<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class A {}
class B {}

function test(string $x): void {
  $y = new A();
  switch ($x) {
    case 'a':
      $y = new B();
      break;
  }
  hh_show($y);
}
