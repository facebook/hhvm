<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class A implements HH\TypeParameterAttribute {
  public function __construct(int $foo) {}
}

class C {
  public function m<<<A>> T>(): void {}
}
