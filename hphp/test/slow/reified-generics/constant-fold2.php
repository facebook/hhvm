<?hh

class C {}
function g(string $s): void { var_dump($s); }
function f<reify T>(): void {
  g(T::class);
  var_dump((string)T::class);
}
<<__EntryPoint>>
function main(): void {
  f<C>();
}
