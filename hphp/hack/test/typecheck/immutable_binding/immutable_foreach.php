<?hh // experimental

function foo(array<int> $numbers): int {
  $sum = 0;
  foreach ($numbers as number) {
    $sum += number;
  }
  return $sum;
}
