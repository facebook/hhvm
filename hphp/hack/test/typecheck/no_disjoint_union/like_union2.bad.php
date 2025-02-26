<?hh

<<file: __EnableUnstableFeatures('no_disjoint_union', 'like_type_hints')>>

class Box<T> {
  public function __construct(private T $x) {}
  public function get(): T {
    return $this->x;
  }
}

function foo<<<__NoDisjointUnion>> T>(T $_, T $_): void {}

function main(~int $like_i, ~string $like_string): void {
  foo($like_i, $like_string);
}
