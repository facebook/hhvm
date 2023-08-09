<?hh

class C {
  public function foo():string { return "A"; }
}

function returnFun():vec<(function(C):arraykey)> {
  return vec[$x ==> $x->foo()];
}
