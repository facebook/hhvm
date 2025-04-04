<?hh

<<__Memoize>>
function memo_fn($a, $b)[leak_safe]: string {
  return "args: $a, $b \n";
}

<<__EntryPoint>>
function main(): mixed{
  echo memo_fn(1, 2);
}
