<?hh

class C<reify T>{}

function f(<<__Soft>> C<int> $x) :mixed{ echo "ok\n"; }
function g($x): <<__Soft>> C<int> { return $x; }
<<__EntryPoint>> function main(): void {
f(new C<string>);
g(new C<string>);
}
