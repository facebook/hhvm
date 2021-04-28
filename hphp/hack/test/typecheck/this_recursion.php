<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface I<-T> { }
class C implements I<C> {
  public function foo(): I<this> {
    return $this;
  }
}
