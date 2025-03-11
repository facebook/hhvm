<?hh

<<file: __EnableUnstableFeatures('no_disjoint_union')>>

function eq<<<__NoDisjointUnion>> T>(T $x, T $y): void {}

function main(?int $i, ?num $n): void {
  eq($i, $n);
}
