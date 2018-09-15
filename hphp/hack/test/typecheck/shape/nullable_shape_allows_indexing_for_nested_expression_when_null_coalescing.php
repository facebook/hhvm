<?hh // strict

function foo(): void {
  $bar = (true ? array('a' => 5) : null);
  $bar['a'] ?? 12;
}

function baz(): void {
  (true ? array('a' => 5) : null)['a'] ?? 12;
}

function qux(bool $condition): void {
  if ($condition) {
    $bar = array('a' => 12);
  } else {
    $bar = null;
  }
  $bar['a'] ?? 3;
}
