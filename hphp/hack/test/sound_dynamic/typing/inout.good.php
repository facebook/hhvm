<?hh

function foo(inout dynamic $d):void { }
function foo2(inout ~int $i):void { }

function bar(dynamic $d, ~vec<int> $v):void {
  foo(inout $d[0]);
  foo2(inout $v[0]);
}
