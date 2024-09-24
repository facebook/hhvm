<?hh

class B {
}
class C extends B {
  public function foo():void { }
}

function expectFunctionB((function(B):void) $f):void { }
function expectFunctionC((function(C):void) $f):void { }

function test1():void {
  // Reject
  expectFunctionB($x ==> $x->foo());
  // Accept
  expectFunctionC($x ==> $x->foo());
  // Reject (not a subtype)
  expectFunctionB((C $x) ==> $x->foo());
  // Accept
  expectFunctionC((C $x) ==> $x->foo());
  // Reject (no method foo)
  expectFunctionB((B $x) ==> $x->foo());
  // Arguable, but should reject, because we're only expecting a B
  expectFunctionC((B $x) ==> $x->foo());
}
