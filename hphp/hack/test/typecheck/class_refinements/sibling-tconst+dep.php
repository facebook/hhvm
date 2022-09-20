<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

final class Ctx<TBox as Box with { type T = T }, T> {
  public function __construct(private TBox $box, private T $t): void {}
}
final class CtxShadyProjection<TBox as Box, T> {
  public function __construct(private TBox $box, private T $t): void
  where TBox::T = T {} // NO ERROR
}

abstract class Box { abstract const type T; }
abstract class BoxWithCtx extends Box {
  const type TCtx = Ctx<this, this::T>; // FIXME(type-refinements) gives error but it shouldn't
  const type TCtxUnsound = CtxShadyProjection<this, this::T>; // OK
}
