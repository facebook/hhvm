<?hh

  class A {
    public static $a = 'A';
    static function f() {
 echo static::$a;
 }
    function g() {
 $this::f();
 }
  }
