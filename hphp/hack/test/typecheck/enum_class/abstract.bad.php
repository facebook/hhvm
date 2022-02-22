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

// wrong type for abstract constant X
enum class F : I extends E {
  D X = new D();
}

// abstract constant in concrete enum class
enum class G: mixed {
  int X = 42;
  abstract string Y;
}

function foo(mixed $_): void {}

// accessing abstract constants
function test(): void {
  foo(E::X);
  foo(G::Y);
}

// missing constant initialization
enum class H : I extends E {}
