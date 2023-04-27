<?hh

class X implements IMemoizeParam {
  public function getInstanceKey()[defaults]: string { return ""; }
}

<<__Memoize(#KeyedByIC)>>
function bad1(X $x)[zoned]: void {}

class Bad1 {
  <<__MemoizeLSB(#KeyedByIC)>>
  public static function bad1LSB(X $x)[zoned]: int { return 0; }
}

class Y implements IMemoizeParam {
  public function getInstanceKey()[zoned_shallow]: string { return ""; }
}

<<__Memoize(#KeyedByIC)>>
function bad2(Y $y)[zoned]: void {}

class Bad2 {
  <<__MemoizeLSB(#KeyedByIC)>>
  public static function bad2LSB(Y $y)[zoned]: int { return 0; }
}
