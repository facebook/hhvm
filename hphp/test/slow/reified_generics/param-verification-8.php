<?hh

class C<reify T> {}

function f<reify T>(C<T> $a) {
  return $a;
}
<<__EntryPoint>> function main(): void {
f<int>(new C<int>);
f<int>(new C<string>);
}
