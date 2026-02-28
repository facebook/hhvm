<?hh

class C {
  public function foo():void { }
}
function expectVecFun(vec<(function(C):void)> $v):void { }

function makeSingle<T>(T $x):vec<T> {
  return vec[$x];
}

function test2(dynamic $d, int $i):void {
  expectVecFun(makeSingle($x ==> $x->foo()));
}
