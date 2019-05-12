<?hh

class C<reify T> {}

function f<reify T>(mixed $x): C<T> {
  return $x;
}
<<__EntryPoint>> function main(): void {
f<int>(new C<int>());
f<int>(new C<string>());
}
