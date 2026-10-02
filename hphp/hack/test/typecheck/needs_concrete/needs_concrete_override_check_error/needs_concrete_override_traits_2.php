<?hh

abstract class C1 {
  public static function m1(): void {}
  public static function m2(): void {}
}

trait Tr {
  <<__NeedsConcrete>>
  public static function m1(): void {}
  <<__NeedsConcrete>>
  public static function m2(): void {}
}

abstract class C2 extends C1 {
  use Tr;
}
