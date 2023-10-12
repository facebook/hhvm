<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class B<-T> {}

interface I {}
function toprun<T>(B<T> $rule, T $rule_input): void {}
function toprunI(B<I> $rule, I $rule_input): void {}

// Derived class
final class D extends B<I> {}

final class TestClass {

  private ?I $mv;

  public static function run<T>(B<T> $rule, T $rule_input): void {}
  public function test(): void {
    // The first two should give similar errors to the last of these
    self::run(new D(), $this->mv);
    //      toprun(new D(), $this->mv);
    //      toprunI(new D(), $this->mv);
  }
}
