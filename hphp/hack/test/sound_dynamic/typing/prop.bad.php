<?hh
  // Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

  class C { }
  class G<T> { }
  class D implements dynamic { }
  class E implements dynamic {
    // This should be rejected, as it is not a subtype of dynamic
    public ?C $x;
    // This should be rejected, as it is not enforceable
    public ?vec<int> $z;
    // This should be rejected: it is not enforceable *and* it's not a subtype of dynamic
    // (Accessibility does not affect things, either)
    private ?G<C> $q;
    // This is safe
    public ?D $y;
  }
