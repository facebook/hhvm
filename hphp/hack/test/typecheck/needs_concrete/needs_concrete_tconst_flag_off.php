<?hh

function capture<reify T>(): void {}

final class Box<reify T> {
  public static function smeth<reify TS>(): void {}
  public function imeth<reify TI>(): void {}
}

abstract class Base {
  <<__Reifiable>> abstract const type T;

  // ok: needs_concrete_abstract_type_const_check is off
  public static function f(Box<int> $b): void {
    HH\ReifiedGenerics\get_class_from_type<this::T>();
    capture<vec<this::T>>();
    new Box<this::T>();
    Box::smeth<this::T>();
    $b->imeth<this::T>();
    $_ = type_structure(static::class, 'T');
  }
}
