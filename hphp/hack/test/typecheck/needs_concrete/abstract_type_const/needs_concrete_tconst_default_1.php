<?hh

abstract class Base {
  abstract const type T = int;

  public static function f(): void {
    // ok: the abstract type constant has a default, so it has a runtime value
    $_ = type_structure(static::class, 'T');
  }
}
