<?hh

class C<reify Ta, reify Tb> {}

function f(C<int, int> $x) :mixed{ var_dump("yep"); }
function g(C<string, int> $x) :mixed{ var_dump("yep"); }
function h(C<string, string> $x) :mixed{ var_dump("yep"); }
<<__EntryPoint>> function main(): void {
f(new C<int, int>);
g(new C<string, int>);
h(new C<string, string>);
}
