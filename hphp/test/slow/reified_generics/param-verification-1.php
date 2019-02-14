<?hh

class C<reify Ta, Tb, reify Tc> {}

function f(C<int, string, int> $x) {}

function g<T>() {
  f(new C<reify int, T, reify string>);
}

g();
