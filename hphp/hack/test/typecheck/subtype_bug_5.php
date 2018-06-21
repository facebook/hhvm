<?hh
class B {}
class C extends B {}

function foo(Pair<mixed, mixed> $x) {}
function test(ImmMap<int, string> $x) { foo($x); }
