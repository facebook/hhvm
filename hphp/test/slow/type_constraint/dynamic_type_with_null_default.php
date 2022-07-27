<?hh

function foo(dynamic $_x = null): void {
  echo "hello\n";
}

<<__EntryPoint>>
function main(): void {
  foo();
  foo(null);
  foo(42);
}
