<?hh

class C<reify T>{}

function f(@C<int> $x) { echo "ok\n"; }
function g($x): @C<int> { return $x; }
<<__EntryPoint>> function main(): void {
f(new C<string>);
g(new C<string>);
}
