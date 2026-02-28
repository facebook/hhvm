<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {

  public static function get_int():int {return 5;}
  public function test(): void {
    $_ = (int $a = A::get_int()): void ==> {};
  }

}
