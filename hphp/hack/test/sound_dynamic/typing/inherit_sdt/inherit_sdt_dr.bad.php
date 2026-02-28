<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__DynamicallyReferenced>>
class C {
  public function foo():int { return 3; }
}

class NonSDT {}

class D extends C {
  // Should be an error cos return does not support dynamic
  public function bar():NonSDT { return new NonSDT(); }
}

// Should be an error because we need E<T> to be SDT whenever C is (i.e. always)
// but default semantics for generic classes is to be SDT if type arguments are
class E<T> extends C {
  // Should be an error cos return *value* does not support dynamic
  public function bar(): mixed {
    return new NonSDT();
  }
}
