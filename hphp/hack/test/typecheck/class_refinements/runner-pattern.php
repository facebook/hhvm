<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

interface Box {
  abstract const type T;
  public function set(this::T $t): void;
}

class Space<T> {
  private vec<T> $v;
  public function __construct() { $this->v = vec[]; }
  public function unwrap(): T { return $this->v[0]; }
}

final class Runner<TBox as Box> {
  public function __construct(private TBox $box) {}

  public function helperShady<T>(Space<T> $in): void
  where T = TBox::T {
    $this->box->set($in->unwrap()); // OK
  }

  public function helper<T>(Space<T> $inv_t): void
  where TBox as Box with { type T = T } {
    $this->box->set($inv_t->unwrap()); // OK
  }

  public function run(): void {
    $inv_t = new Space(); // Note: T is inferred
    $this->helperShady($inv_t); // OK
    $this->helper($inv_t); // FIXME(type-refinements) must type-check
  } // but Hack gives error `expected an object with `type T = _` but got `TBox as Box`

  public function runWorkaround<T>(): void
  where TBox as Box with { type T = T } {
    $inv_t = new Space(); // Note: T is inferred
    $this->helper($inv_t); // OK
  }
}
