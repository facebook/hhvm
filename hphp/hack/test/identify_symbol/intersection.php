<?hh

// TODO(t11082787): multiple definitions of a symbol

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
  // this is both C1::foo() and C2::foo(), currently will choose the last one
  $x->foo();
}
