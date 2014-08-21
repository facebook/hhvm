<?hh
class B {}
class C extends B {}

function foo(ConstVector<mixed> $x) {}
function test(ConstSet<string> $x) { foo($x); }
