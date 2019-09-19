<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class A {
  public function f(): void where int as string {}
}

class B extends A {}
