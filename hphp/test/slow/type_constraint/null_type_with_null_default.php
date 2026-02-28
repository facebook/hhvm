<?hh

function foo(null $_x = null): void {
  echo "hello\n";
}

<<__EntryPoint>>
function main(): void {
  foo();
  foo(null);
  foo(1);
}
