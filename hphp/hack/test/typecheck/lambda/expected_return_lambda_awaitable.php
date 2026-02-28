<?hh

class C {
  public function foo():void { }
}
function expectVecFun(vec<(function(C):Awaitable<void>)> $v):void { }

function makeSingle<T>(T $x):vec<T> {
  return vec[$x];
}

function test2(dynamic $d, int $i):void {
  expectVecFun(makeSingle(async $x ==> $x->foo()));
}
