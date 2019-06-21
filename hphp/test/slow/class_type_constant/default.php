<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class C {
  abstract const type T = arraykey;
}

class D extends C {

}

class E extends C {
  const type T = int;
}

var_dump(type_structure(D::class, 'T'));
var_dump(type_structure(E::class, 'T'));
