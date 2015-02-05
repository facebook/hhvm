<?hh

function test((function(int): bool) $f): void {
  array_map($f, $f);
}
