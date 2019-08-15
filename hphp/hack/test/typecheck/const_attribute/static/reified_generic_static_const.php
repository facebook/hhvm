<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class MyTest {
  <<__Const>> public static int $i = 4;
}

function f<reify T as MyTest>(): void {
  T::$i = 5; // should error
}
