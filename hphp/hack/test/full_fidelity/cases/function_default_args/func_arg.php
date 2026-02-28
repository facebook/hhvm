<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {

  public static function get_int():int {return 5;}
  public function take_int(int $a = A::get_int()): void{}

}
