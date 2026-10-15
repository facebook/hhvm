<?hh

function capture<reify T>(): void {}

final class Box<reify T> {
  public static function smeth<reify TS>(): void {}
  public function imeth<reify TI>(): void {}
}

abstract class Base {
  <<__Reifiable>> abstract const type T;

  public static function nested(): void {
    // error: nested in a reified type argument
    capture<vec<this::T>>();
  }

  public static function constructor(): void {
    // error: reified class type argument
    new Box<this::T>();
  }

  public static function static_method(): void {
    // error: reified static method type argument
    Box::smeth<this::T>();
  }

  public static function instance_method(Box<int> $b): void {
    // error: reified instance method type argument
    $b->imeth<this::T>();
  }

  <<__NeedsConcrete>>
  public static function ok(Box<int> $b): void {
    capture<vec<this::T>>();
    new Box<this::T>();
    Box::smeth<this::T>();
    $b->imeth<this::T>();
  }
}
