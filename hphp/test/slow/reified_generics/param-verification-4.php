<?hh

class A<T> {}
class B<reify T> {}
class C<reify Ta, Tb, reify Tc> {}

function f(B<A<B<int>>> $_) { echo "yep\n"; }
f(new B<A>());

function g(A<B<int>> $_) { echo "yep\n"; }
g(new A());

function h(C<A<int>, string, int> $_) { echo "yep\n"; }
h(new C<A<int>, string, int>());

function i(B<C<int, string, int>> $_) { echo "yep\n"; }
i(new B<C<int, int, int>>());

function j(C<A<int>, string, B<C<int, string, int>>> $_) { echo "yep\n"; }
j(new C<A<int>, string, B<C<int, int, int>>>());
