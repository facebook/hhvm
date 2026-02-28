<?hh

enum class E : mixed {
  int X = 42;
  string Y = 'yolo';
}

function g(?HH\EnumClass\Label<E, int> $label): int {
  if ($label is nonnull) {
    return E::valueOf($label);
  } else {
    return 0;
  }
}

<<__EntryPoint>>
function run(): void {
  $x = g(null);
  echo $x;
  echo "\n";

  $x = g(#X);
  echo $x;
  echo "\n";
}
