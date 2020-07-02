<?hh

function foo<T>(T $arg): void {
  echo "done\n";
}

<<__EntryPoint>>
function test(): void {
  $x = foo<>;
  $x(10);
}
