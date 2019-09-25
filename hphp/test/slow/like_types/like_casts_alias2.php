<?hh

function f(mixed $m): int {
  $li = $m as HH\INCORRECT_TYPE<int>;

  return $li;
}

<<__EntryPoint>>
function main(): void {
  f("3");
}
