<?hh

class Foo {
  public static $z = 0;

  public static function setZ($a) {
    Foo::$z = $a;
  }

  public static function getZ() {
    return Foo::$z;
  }
}
<<__EntryPoint>> function main(): void {
Foo::setZ(4);

var_dump(Foo::getZ());
}
