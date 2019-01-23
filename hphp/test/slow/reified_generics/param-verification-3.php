<?hh

class C<reify Ta, Tb, reify Tc> {}

function f(C<int, string, int> $x) { var_dump("yep"); }
function g(C<string, string, int> $x) { var_dump("yep"); }
function h(C<string, string, string> $x) { var_dump("yep"); }

f(new C<reify int, int, reify int>);
g(new C<reify string, int, reify int>);
h(new C<reify string, int, reify string>);
