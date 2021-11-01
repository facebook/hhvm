<?hh
class B {}
class C extends B {}

function foo(ImmMap<int, B> $x): void {}
function test(ConstMap<int, C> $x): void { foo($x); }
