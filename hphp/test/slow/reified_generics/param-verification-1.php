<?hh

class C<reify Ta, Tb, reify Tc> {}

function f(C<int, string, int> $x) {}

f(new C<reify int, int, reify string>);
