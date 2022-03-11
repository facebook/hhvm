<?hh

function foo(inout ~int $i):void { }
function bar(~Vector<int> $v):void {
  foo(inout $v[0]);
}
