<?hh

class C {
  public static function meth<T1,T2>(T1 $x, vec<T2> $y):T1 {
    return $x;
  }
}

function testit():void {
  $y = C::meth(2, vec["A"]);
  //       ^ hover-at-caret
}
