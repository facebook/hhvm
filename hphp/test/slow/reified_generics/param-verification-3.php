<?hh

class C<reify Ta, Tb, reify Tc> {}

function f(C<int, string, int> $x) { var_dump("yep"); }
function g(C<string, string, int> $x) { var_dump("yep"); }
function h(C<string, string, string> $x) { var_dump("yep"); }

function a<T>() {
  f(new C<reify int, T, reify int>);
  g(new C<reify string, T, reify int>);
  h(new C<reify string, T, reify string>);
}

a();
