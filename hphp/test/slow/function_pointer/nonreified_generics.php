<?hh

function foo<T, Ta>(T $arg): void {
  echo "done\n";
}

<<__EntryPoint>>
function test(): void {
  $x = foo<int, _>;
  $x(4);
}
