<?hh

class C<reify T1, T2, reify T3> {}

function f<reify T1, T2, reify T3>(int $x) {}

function g() {
  f<reify int, string, reify bool>(1);
  new C<reify int, string, reify bool>();
}
