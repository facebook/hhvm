<?hh

<<file: __EnableUnstableFeatures('no_disjoint_union')>>

function foo<<<__NoDisjointUnion>> T>(T $_, vec<T> $_): void {}

function main(arraykey $a, num $n): void {
  // Not marked as disjoint because we look for pairwise disjointness
  foo(true, vec[$a, $n]);
  foo($n, vec[true, $a]);
}
