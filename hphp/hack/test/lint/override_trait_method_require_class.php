<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<file:__EnableUnstableFeatures('require_class')>>

final class A implements HH\MethodAttribute {}
final class AnotherAttribute implements HH\MethodAttribute {}

trait T {
  require class C;

  public function overridden(): void {}
  public function notOverridden(): void {}
  <<A>>
  public function hasAttribute1(): void {}
  <<AnotherAttribute>>
  public function hasAttribute2(): void {}
  <<A>>
  abstract public function hasAttributeAbstract1(): void;
  <<AnotherAttribute>>
  abstract public function hasAttributeAbstract2(): void;
  <<__Memoize>>
  public function hasBuiltInAttribute(): void {}
}

class C {
  use T;

  <<__Override>>
  public function overridden(): void {}

  <<__Override>>
  public function hasAttribute1(): void {}
  <<__Override>>
  public function hasAttribute2(): void {}

  <<__Override>>
  public function hasAttributeAbstract1(): void {}
  <<__Override>>
  public function hasAttributeAbstract2(): void {}

  <<__Override>>
  public function hasBuiltInAttribute(): void {}
}
