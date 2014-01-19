<?php

trait T {
  private static function blarg() {
    echo "T::blarg\n";
  }
  private static function test() {
    echo __CLASS__ . "\n";
    echo get_called_class() . "\n";
    self::blarg();
  }
  public static function doTest() {
    self::test();
  }
}
class C {
  use T;
}
class D extends C {
  protected static function test() {
    echo "D::test\n";
  }
  public static function doTest() {
    parent::doTest();
  }
}
D::doTest();
echo "Done\n";
