<?hh

<<file: __EnableUnstableFeatures('no_disjoint_union')>>

class Box<T> {
  public function __construct(private T $x) {}
  public function get(): T {
    return $this->x;
  }
}

function foo<<<__NoDisjointUnion>> T>(T $_, T $_): void {}

function main(): void {
  // N.b. without the type argument on `Box`, `foo` gets a `mixed` type
  // argument, hence we miss the disjointness warning.
  $like_i = (new Box<int>(42))->get();
  foo($like_i, 'a');
}
