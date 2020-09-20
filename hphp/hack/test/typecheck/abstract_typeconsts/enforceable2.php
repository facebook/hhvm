<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class C {
  <<__Enforceable>>
  abstract const type T as vec<arraykey>;
}

abstract class D extends C {
  abstract const type T as vec<arraykey> = vec<int>;
}
