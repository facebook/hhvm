<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public function Boo(): void {
    echo 'boo';
  }
}
class B {}

function Foo(bool $b, bool $c, mixed $m, A $i): ?A {
  $res = null;
  if ($b) {
    if ($c) {
      $res = $m;
    }
  } else if ($c) {
    $res = $i;
  }
  return $res;
}

function BreakIt(): void {
  $b = new B();
  $opta = Foo(true, true, $b, new A());
  if ($opta !== null) {
    $opta->Boo();
  }
}

//BreakIt();
