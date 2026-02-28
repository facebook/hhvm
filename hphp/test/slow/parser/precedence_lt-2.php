<?hh

function i<T>(T $i): T {
  return $i;
}

<<__EntryPoint>>
function f(): void {
  echo(10 - i<int>(1) - 1);
}
