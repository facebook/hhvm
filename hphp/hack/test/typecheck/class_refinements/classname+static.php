<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

abstract class Box {
  abstract const type T;
  public function m(): void {
    f(static::class); // OK
    f(self::class); // ERROR
  }
}

function f<TBox as Box with { type T = T }, T>(
  classname<TBox> $cls_box,
): void {}
