<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function cmeth<T as arraykey>(T $x):void { }
  public static function smeth<T as arraykey>(T $x):void { }
}
class D<Td> {
  public function cmeth_where(Td $x):void where Td as arraykey  { }
  public static function smeth_where(Td $x):void where Td as arraykey { }
}

function testit():void {
  $c = new C();
  $d = new D<float>();
  $c->cmeth(2.3);
  C::smeth(true);
  $d->cmeth_where(2.3);
  D::smeth_where(true);
}
