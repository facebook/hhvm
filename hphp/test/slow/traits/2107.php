<?php

trait T {
  public static function foo() {
    $bt = debug_backtrace();
    echo $bt[0]['function'] . "\n";
  }
}
class C {
  use T {
    T::foo as public bar1;
    T::foo as public bar2;
  }
}
C::bar1();
C::bar2();
