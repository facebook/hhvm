<?hh

function foo(string $a, int $b, bool $c): void {
  var_dump($a, $b, $c);
}

<<__EntryPoint>>
function main(): void {
  $b = tuple('hello', tuple(42, true));
  list($a, $b) = $b;
  foo($a, $b[0], $b[1]);
}
