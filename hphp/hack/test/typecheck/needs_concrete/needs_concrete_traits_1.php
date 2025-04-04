<?hh

trait Tr {
  <<__NeedsConcrete>>
  public static function m1(): void {
    static::abs();
  }
  public static abstract function abs(): void;
}

abstract class C1 {
  use Tr;
}

function example(): void {
  C1::m1(); // hh error
}
