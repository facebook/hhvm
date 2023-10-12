<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class B {}
class C {
  public function foo(): void {
    echo 'foo';
  }
}
function f(int $c): C {
  $x = new C();
  switch ($c) {
    case 1:
      if (true) {
        $x = new B();
        break;
      }
      $x = new C();
      // FALLTHROUGH
    case 2:
      break;
  }

  return $x;
}
