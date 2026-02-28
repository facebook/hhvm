<?hh

<<file: __EnableUnstableFeatures('no_disjoint_union')>>

class C<<<__NoDisjointUnion>> T> {
  public function __construct(T $_, T $_) {}
}

function main(int $i, arraykey $n): void {
  new C($i, $n);
}
