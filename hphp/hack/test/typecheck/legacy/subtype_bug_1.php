<?hh
class B {}
class C extends B {}

function foo(ImmVector<B> $x) {}
function test(ConstVector<C> $x) { foo($x); }
