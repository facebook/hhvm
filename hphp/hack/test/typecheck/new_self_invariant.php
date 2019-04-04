<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C<Tc> {}

class D<Td as int> extends C<Td> {
  public function f(): D<int> {
    return new self();
  }
}
