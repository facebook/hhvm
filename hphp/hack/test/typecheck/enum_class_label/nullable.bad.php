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
  $x = g(#Y);
  echo $x;
  echo "\n";

  $x = g(#Z);
  echo $x;
  echo "\n";
}
