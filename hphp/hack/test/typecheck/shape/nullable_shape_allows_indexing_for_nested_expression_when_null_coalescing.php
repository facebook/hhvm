<?hh

function foo(): void {
  $bar = (true ? darray['a' => 5] : null);
  $bar['a'] ?? 12;
}

function baz(): void {
  (true ? darray['a' => 5] : null)['a'] ?? 12;
}

function qux(bool $condition): void {
  if ($condition) {
    $bar = darray['a' => 12];
  } else {
    $bar = null;
  }
  $bar['a'] ?? 3;
}
