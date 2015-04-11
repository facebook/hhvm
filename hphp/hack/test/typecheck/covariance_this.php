<?hh

// this test ensures that our variance checks don't raise a false positive on
// `this`, which is implemented as a special `as` type constraint
class Foo<+T> {
  public function bar(): this {
    // UNSAFE
  }
}
