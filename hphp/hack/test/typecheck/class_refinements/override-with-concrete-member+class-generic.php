<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

interface Box {
  abstract const type T;
}
abstract class Base<TBox as Box> {
  abstract protected function mShady<T>(T $_): void
  where TBox::T = T;

  abstract protected function m<T>(T $_): void
  where TBox as Box with { type T = T };
}

class ConcreteIntBox implements Box {
  const type T = int;
}
final class Child extends Base<ConcreteIntBox> {

  <<__Override>>
  protected function mShady(int $_): void {} // NO ERROR

  <<__Override>>
  protected function m(int $_): void {} // NO ERROR
}
