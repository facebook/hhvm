<?php
class A5 {
  private function A5() {
  }
}
class B5 extends A5 {
  public static function test() {
    return new A5;
  }
}
B5::test();
