<?hh // partial

function test(Container<int> $x): void {
  array_map($x, $x);
}
