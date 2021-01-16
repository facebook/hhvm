<?hh

class C {
  public function __clone()[int]: void {}
}

class D {}

function test(C $c, D $d)[string]: void {
  clone $c;
  clone $d; // undefined __clone methods are typed as pure
}
