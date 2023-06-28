<?hh

class A<T> {}
class B<reify T> {}
class C<reify Ta, Tb, reify Tc> {}

function f(B<A<B<int>>> $_) :mixed{ echo "yep\n"; }

function g(A<B<int>> $_) :mixed{ echo "yep\n"; }

function h(C<A<int>, string, int> $_) :mixed{ echo "yep\n"; }

function i(B<C<int, string, int>> $_) :mixed{ echo "yep\n"; }

function j(C<A<int>, string, B<C<int, string, int>>> $_) :mixed{ echo "yep\n"; }
<<__EntryPoint>>
function main_entry(): void {
  f(new B<A>());
  g(new A());
  h(new C<A<int>, string, int>());
  i(new B<C<int, int, int>>());
  j(new C<A<int>, string, B<C<int, int, int>>>());
}
