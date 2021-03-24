<?hh
  // Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

  class C { }
  class G<T> { }

  <<__SoundDynamicCallable>>
  class D { }
  <<__SoundDynamicCallable>>
  class E {
    // This should be rejected, as it is not a subtype of dynamic
    public ?C $x;
    // This should be rejected, as it is not enforceable
    public ?vec<int> $z;
    // This should not be rejected: although it's not enforceable or a subtype
    // of dynamic, it's private. We check possible dynamic access to such
    // properties within code, not here at the definition.
    private ?G<C> $q;
    // This is safe
    public ?D $y;
  }
