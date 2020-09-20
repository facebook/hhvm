<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Good implements HH\TypeConstantAttribute {
  public function __construct(public int $i) {}
}

class Bad implements HH\ClassAttribute {
  public function __construct(public string $s) {}
}

abstract class C {
  <<__Enforceable>>
  const type T = int;

  <<Good(4)>>
  abstract const type Tu;

  <<Bad("hi")>>
  abstract const type Tv as int;
}
