<?hh // partial
class B {}
class C extends B {}

function foo(ImmSet<arraykey> $x) {}
function test(ConstSet<string> $x) { foo($x); }
