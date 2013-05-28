<?php

trait T {
  public static function F() {
    echo "Hello from static function!\n";
  }
}
class C {
  use T;
}
C::F();
?>
