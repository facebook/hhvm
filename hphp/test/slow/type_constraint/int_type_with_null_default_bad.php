<?hh

function foo(int $_x = null): void {
  echo "hello\n";
}

<<__EntryPoint>>
function main(): void {
  foo();
}
