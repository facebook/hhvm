<?hh

abstract class A {
  <<__NeedsConcrete>>
  public static function m1(): void {
    static::m2(); // ok: the attribute ensures `static` refers to a concrete class
  }

  public static abstract function m2(): void;
}

<<__EntryPoint>>
function main(): void {
  A::m1(); // hh error and runtime error
}
