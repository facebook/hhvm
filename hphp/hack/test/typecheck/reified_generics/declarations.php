<?hh

class C<reify T1, T2, reify T3> {}

function f<reify T1, T2, reify T3>(int $x): void {}

function g(): void {
  f<int, string, bool>(1);
  new C<int, string, bool>();
}
