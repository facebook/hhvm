<?hh
class B {}
class C extends B {}

function foo(Pair<mixed, mixed> $x): void {}
function test(ImmMap<int, string> $x): void { foo($x); }
