<?hh

class A {
  <<__LSB>> private static $x = "hello";
  <<__LSB>> protected static $y = 123;
  <<__LSB>> public static $z = vec[1,2,3];

  static function dump() :mixed{
    var_dump(static::$x);
    var_dump(static::$y);
    var_dump(static::$z);
    static::$x = "world";
    static::$y = 1234;
    static::$z[] = 4;
  }
}

class B extends A {
}

class C extends B {
}
<<__EntryPoint>> function main(): void {
A::dump();
B::dump();
C::dump();

A::dump();
B::dump();
C::dump();
}
