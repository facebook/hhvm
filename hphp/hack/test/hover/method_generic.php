<?hh

class C {
  public function meth<T1,T2>(T1 $x, vec<T2> $y):T1 {
    return $x;
  }
}

function testit():void {
  $c = new C();
  $y = $c->meth(2, vec["A"]);
  //       ^ hover-at-caret
}
