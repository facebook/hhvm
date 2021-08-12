<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('abstract_enum_class')>>

interface I {}
class C implements I {}
class D implements I {}
class CC extends C {}

abstract enum class E : I {
  abstract C X;
  D Y = new D();
}

enum class F : I extends E {
  C X = new C();
}

abstract enum class G : I extends F {
  abstract C Z;
}

enum class H0 : I extends G {
  C Z = new C();
}

enum class H1 : I extends G {
  CC Z = new CC();
}
