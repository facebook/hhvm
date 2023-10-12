<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class C<T as nothing> {
  abstract const nothing NOTHING;
  private ?nothing $null = null;
  private Vector<nothing> $empty_vector = Vector {};
  public function f(nothing $x): nothing {
    return $x;
  }
}
