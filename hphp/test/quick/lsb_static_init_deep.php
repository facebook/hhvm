<?hh

class A {
  <<__LSB>> private static $x = Vector{"hello", "world"};
  <<__LSB>> protected static $y = Set{1, 2, 3};
  <<__LSB>> public static $z = Map{1 => "foo", 2 => "bar", 3 => "baz"};

  static function dump() :mixed{
    var_dump(static::$x);
    var_dump(static::$y);
    var_dump(static::$z);
    static::$x->clear();
    static::$y->clear();
    static::$z->clear();
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
