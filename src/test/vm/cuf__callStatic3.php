<?php

class A {
  private static function private_func () {
    echo "In private_func!\n";
  }
  protected static function protected_func () {
    echo "In protected_func!\n";
  }
  public static function public_func () {
    echo "In public_func!\n";
  }
  static function __callStatic($f, $a) {
    echo "A::__callStatic: " . $f . "(" . $a[0] . "," . $a[1] . ")\n";
  }
  public static function test() {
    call_user_func(array('A', 'private_func'), "1", "2", "3");
    call_user_func(array('A', 'protected_func'), "1", "2", "3");
    call_user_func(array('A', 'public_func'), "1", "2", "3");
  }
}

class B extends A {
  static function __callStatic($f, $a) {
    echo "B::__callStatic: " . $f . "(" . $a[0] . "," . $a[1] . ")\n";
  }

  public static function test() {
    call_user_func(array('A', 'private_func'), "1", "2", "3");
    call_user_func(array('A', 'protected_func'), "1", "2", "3");
    call_user_func(array('A', 'public_func'), "1", "2", "3");
  }
}


function main() {
  call_user_func(array('A', 'private_func'), "1", "2", "3");
  call_user_func(array('A', 'protected_func'), "1", "2", "3");
  call_user_func(array('A', 'public_func'), "1", "2", "3");

  call_user_func(array('B', 'private_func'), "1", "2", "3");
  call_user_func(array('B', 'protected_func'), "1", "2", "3");
  call_user_func(array('B', 'public_func'), "1", "2", "3");

  A::test();
  B::test();
}
main();

