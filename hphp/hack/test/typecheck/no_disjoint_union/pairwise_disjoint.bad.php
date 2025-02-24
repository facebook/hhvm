<?hh

<<file: __EnableUnstableFeatures('no_disjoint_union')>>

function foo<<<__NoDisjointUnion>> T>(T $_, vec<T> $_): void {}

function main(): void {
  foo(true, vec[42, 'foo']);
}
