<?hh

function i(int $i): int {
  return $i;
}

<<__EntryPoint>>
function f(): void {
  echo(10 - i<>(1) - 1);
}
