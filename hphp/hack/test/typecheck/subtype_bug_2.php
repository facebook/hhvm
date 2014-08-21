<?hh
class B {}
class C extends B {}

function foo(ImmMap<int, B> $x) {}
function test(ConstMap<int, C> $x) { foo($x); }
