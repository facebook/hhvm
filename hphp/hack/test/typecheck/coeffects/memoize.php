<?hh

class X implements IMemoizeParam {
  public function getInstanceKey()[defaults]: string { return ""; }
}

<<__Memoize>>
function pure(X $x)[]: void {}
