<?hh

class C {}
function f(string $s, string $t): void {}

<<__EntryPoint>>
function main(): void {
  f(
    "hello",
    C::class,
  );
}
