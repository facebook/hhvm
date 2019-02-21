<?hh // partial

class C<reify T1, T2, reify T3> {}

function f<reify T1, T2, reify T3>(int $x) {}

function g() {
  f<int, string, bool>(1);
  new C<int, string, bool>();
}
