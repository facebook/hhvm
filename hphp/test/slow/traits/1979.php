<?php

trait T1 {
  static function hello() {
    echo "Hello ";
    self::world();
  }
}
trait T2 {
  use T1;
  static function world() {
    echo "World!\n";
  }
}
class C {
  use T2;
  static function p() {
    self::hello();
    self::world();
  }
}
C::p();
?>
