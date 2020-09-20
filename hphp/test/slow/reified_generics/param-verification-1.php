<?hh

class C<reify Ta, Tb, reify Tc> {}

function f(C<int, string, int> $x) {}

function g<T>() {
  f(new C<int, T, string>);
}
<<__EntryPoint>> function main(): void {
g();
}
