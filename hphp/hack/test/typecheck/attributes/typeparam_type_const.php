<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class A implements HH\TypeParameterAttribute {
  public function __construct(int $foo) {}
}

class C {
  const type Ty<<<A>> T> = vec<int>;
}
