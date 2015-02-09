<?hh

function test((function(int, string): bool) $f): void {
  array_map($f);
}
