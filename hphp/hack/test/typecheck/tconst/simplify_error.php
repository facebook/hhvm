<?hh

abstract class P<T> {
  abstract protected function foo(): T;
}

abstract class C extends P<this::T> {
  abstract const type T;
  // Silence the Naming phase errors so we see the
  // unification error
  /* HH_IGNORE_ERROR[2002] */
  /* HH_IGNORE_ERROR[2049] */
  abstract protected function foo(): T;
}
