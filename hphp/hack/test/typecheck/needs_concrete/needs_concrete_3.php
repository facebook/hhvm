<?hh

abstract class C1 {
  <<__NeedsConcrete>>
  public static function m2(): void {
    static::m3();
  }
  public static abstract function m3(): void;

}

abstract class C2 extends C1 {
  public static function m1(): void {
    // hh error: unsafe because `static` may refer to a non-concrete class
    parent::m2();
  }
}

class C3 extends C2 {
  public static function m3(): void {
    echo "m3\n";
  }
}

<<__EntryPoint>>
function main(): void {
  C3::m1(); // prints "m3"
}
