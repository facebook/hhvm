<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class C {
  abstract const type T = arraykey;
}

class D extends C {

}

class E extends C {
  const type T = int;
}
<<__EntryPoint>> function main(): void {
var_dump(type_structure(D::class, 'T'));
var_dump(type_structure(E::class, 'T'));
}
