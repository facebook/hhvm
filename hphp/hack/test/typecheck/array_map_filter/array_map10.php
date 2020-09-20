<?hh // strict

function test<TA, TB, TC>((function(TA): TB) $f, vec<TC> $x): void {
  array_map($f, $x);
}
