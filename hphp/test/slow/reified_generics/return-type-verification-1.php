<?hh

class C<reify T> {}

function f(mixed $x): C<int> {
  return $x;
}
<<__EntryPoint>> function main(): void {
f(new C<int>());
f(new C<string>());
}
