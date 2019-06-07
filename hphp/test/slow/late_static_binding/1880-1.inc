<?hh

  class A {
    static $a = 'A';
    static function f() {
 echo static::$a;
 }
    function g() {
 $this::f();
 }
  }
  $x = 1;
