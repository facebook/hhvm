<?hh
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
  // we expect (exact B) since the switch will
  // throw when $x !== 'a'
  hh_show($y);
}
