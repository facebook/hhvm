<?hh
class B {}
class C extends B {}

function foo(ConstVector<mixed> $x): void {}
function test(ConstSet<string> $x): void { foo($x); }
