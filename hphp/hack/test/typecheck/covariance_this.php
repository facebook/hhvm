<?hh // partial

// this test ensures that our variance checks don't raise a false positive on
// `this`, which is implemented as a special `as` type constraint
class Foo<+T> {
  /* HH_FIXME[4336] */
  public function bar(): this {
  }
}
