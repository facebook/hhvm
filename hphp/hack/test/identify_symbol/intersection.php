<?hh

class C1 {
  public function foo() {}
}

class C2 {
  public function foo() {}
}

function test(C1 $c1, C2 $c2, bool $b) {
  if ($b) {
    $x = $c1;
  } else {
    $x = $c2;
  }
  $x->foo();
}
