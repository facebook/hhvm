<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

trait T1 {
  <<__Const>> public static num $p = 5.4;
}

trait T2 {
  <<__Const>> public static num $p = 5.0;
}

trait T3 {
  <<__Const>> public static num $p = 4;
}

class C {
  use T1, T2, T3;
}
