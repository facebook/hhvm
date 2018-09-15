<?hh // strict

abstract class C {
  abstract const type T as nonnull;

  public function f(this::T $x): nonnull {
    return $x;
  }
}
