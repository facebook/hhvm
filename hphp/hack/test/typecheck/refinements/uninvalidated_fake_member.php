<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Fooish {
  public function foo():void { }
}
class C {
  public function __construct(public mixed $m) { }
}
function testit():void {
  $cfooish = new C(new Fooish());
  $cstring = new C("a");
  if ($cfooish->m is Fooish) {
    $cfooish = $cstring;
    $cfooish->m->foo();
  }
}
