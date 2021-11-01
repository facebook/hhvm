<?hh
class B {}
class C extends B {}

function foo(ImmSet<arraykey> $x): void {}
function test(ConstSet<string> $x): void { foo($x); }
