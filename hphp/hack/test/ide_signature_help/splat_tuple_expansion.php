<?hh

class Splat<Targs as (mixed...)> {}

final class TwoInts extends Splat<(int, int)> {}

class OtherClass {
  /** Generic instance method with type splat */
  public static function genericSplatMethod<Targs as (mixed...)>(
    Splat<Targs> $splat,
    ... Targs $args,
  ): void {}
}

function test(): void {
  OtherClass::genericSplatMethod(new TwoInts(), 1, 2);
  //                                              ^ at-caret
}
