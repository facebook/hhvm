<?hh

class C<reify T1, T2, reify T3> {}

function g<T>() {
  new C<int, string, T>();
}
<<__EntryPoint>> function main(): void {
g();
}
