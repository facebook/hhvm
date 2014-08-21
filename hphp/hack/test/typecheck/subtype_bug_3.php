<?hh
class B {}
class C extends B {}

function foo(ImmSet<mixed> $x) {}
function test(ConstSet<string> $x) { foo($x); }
