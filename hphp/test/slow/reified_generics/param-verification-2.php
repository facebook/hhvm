<?hh

class C<reify Ta, reify Tb> {}

function f(C<int, int> $x) { var_dump("yep"); }
function g(C<string, int> $x) { var_dump("yep"); }
function h(C<string, string> $x) { var_dump("yep"); }

f(new C<reify int, reify int>);
g(new C<reify string, reify int>);
h(new C<reify string, reify string>);
