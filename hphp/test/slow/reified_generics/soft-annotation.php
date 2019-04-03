<?hh

class C<reify T>{}

function f(@C<int> $x) { echo "ok\n"; }
function g($x): @C<int> { return $x; }

f(new C<string>);
g(new C<string>);
