<?hh

<<file: __EnableUnstableFeatures('no_disjoint_union')>>

function equal<<<__NoDisjointUnion>> T>(T $t1, T $t2): bool {
  return $t1 === $t2;
}

function main(int $i, string $j): void {
  equal($i, $j);
}
