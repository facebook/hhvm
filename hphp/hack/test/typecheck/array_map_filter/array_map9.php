<?hh // strict

function test<TA, TB>(TA $a, TB $b): void {
  array_map($x ==> $x, vec[$a, $b]);
}
