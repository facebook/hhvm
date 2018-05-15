<?hh // strict

abstract class C {
  abstract const type T as (function(): void);

  public static function isT(mixed $x): void {
    if ($x is this::T) {
    }
  }
}
