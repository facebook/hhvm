<?hh

function bar<T>(T $x = null): void where T = ?int {
  echo "hello\n";
}

<<__EntryPoint>>
function main(): void {
  bar();
  bar(null);
  bar(42);
}
