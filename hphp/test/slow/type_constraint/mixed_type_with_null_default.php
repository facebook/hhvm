<?hh

function foo(mixed $_x = null): void {
  echo "hello\n";
}

<<__EntryPoint>>
function main(): void {
  foo();
  foo(null);
  foo(42);
}
