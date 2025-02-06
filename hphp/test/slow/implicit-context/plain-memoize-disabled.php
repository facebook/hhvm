<?hh

<<__Memoize>>
function memo_fn($a, $b): void {
  echo "args: $a, $b \n";
}

<<__EntryPoint>>
function main(): mixed{
  memo_fn(1, 2);
}
