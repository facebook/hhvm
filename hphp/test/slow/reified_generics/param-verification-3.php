<?hh

class C<reify Ta, Tb, reify Tc> {}

function f(C<int, string, int> $x) { var_dump("yep"); }
function g(C<string, string, int> $x) { var_dump("yep"); }
function h(C<string, string, string> $x) { var_dump("yep"); }

function a<T>() {
  f(new C<int, T, int>);
  g(new C<string, T, int>);
  h(new C<string, T, string>);
}
<<__EntryPoint>> function main(): void {
a();
}
