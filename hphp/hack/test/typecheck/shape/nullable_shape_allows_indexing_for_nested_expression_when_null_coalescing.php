<?hh

function foo(): void {
  $bar = (true ? dict['a' => 5] : null);
  $bar['a'] ?? 12;
}

function baz(): void {
  (true ? dict['a' => 5] : null)['a'] ?? 12;
}

function qux(bool $condition): void {
  if ($condition) {
    $bar = dict['a' => 12];
  } else {
    $bar = null;
  }
  $bar['a'] ?? 3;
}
