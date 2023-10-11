<?hh

abstract class C {
  abstract const type T;

  abstract const classname<this::T> C;

  public function f(): nonnull {
    $c = static::C;
    return new $c();
  }
}
