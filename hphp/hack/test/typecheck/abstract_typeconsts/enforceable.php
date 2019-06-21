<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class C {
  <<__Enforceable>>
  const type T1 = vec<int>;

  <<__Enforceable>>
  abstract const type T2 = vec<int>;

  <<__Enforceable>>
  const type T3 as vec<arraykey> = vec<int>;
}
