<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function bar(int $x, string $y):void { }

class C {
  public static function foo(int $x, inout string $y):void { }
}

function testit():void {
  $v = darray[
    'a' => class_meth(C::class, 'foo'),
    'b' => fun('bar')
  ];
  /* HHFIXME[4104] */
  $v['a'](3, 'b');
}
