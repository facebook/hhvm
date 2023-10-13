<?hh

abstract class C {
  abstract const type T;

  public function f(this::T $x): nonnull {
    return $x;
  }
}
