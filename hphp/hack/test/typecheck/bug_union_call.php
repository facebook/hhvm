<?hh

function test(bool $b, Map<int, int> $n): void {
  if ($b) {
    $m = ($b ? null : $n);
  } else {
    $m = Map {0 => false};
  }
  $m?->get(0);
}
