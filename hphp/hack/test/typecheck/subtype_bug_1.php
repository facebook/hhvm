<?hh
class B {}
class C extends B {}

function foo(ImmVector<B> $x): void {}
function test(ConstVector<C> $x): void { foo($x); }
