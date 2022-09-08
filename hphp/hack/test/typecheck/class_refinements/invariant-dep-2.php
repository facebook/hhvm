<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

interface Invariant<T> { public function unwrap(): T; }

interface Box {
  abstract const type T;
  public function get(): this::T;
}
interface ArraykeyBox extends Box {
  abstract const type T as arraykey;
}
class ConcreteIntBox implements ArraykeyBox {
  const type T = int;
  public function get(): this::T { return 0; }
}

abstract class Outer<TBox as ArraykeyBox> {
  abstract const type TBT as arraykey;
  abstract const type TBox as Box;
  public abstract function boxGet<T as arraykey>(
    Invariant<TBox> $inv_box
  ) : T
    where TBox as Box with { type T = T };
}
class ConcreteOuterGood1 extends Outer<self::TBox> {
  const type TBT = int;
  const type TBox = ConcreteIntBox;

  <<__Override>>
  public function boxGet(
    Invariant<self::TBox> $inv_box
  ): int // ERROR: expected T but got int
    // with or without where-clause, doesn't really matter:
    // where self::TBox as Box with { type T = int }
  {
    return $inv_box->unwrap()->get();
  }
}

class ConcreteOuterGood2 extends Outer<self::TBox> {
  const type TBT = int;
  const type TBox = ConcreteIntBox;

  <<__Override>>
  public function boxGet<T2 as int>(
    Invariant<self::TBox> $inv_box
  ): T2
    // with or without where-clause, doesn't really matter:
    // where self::TBox as Box with { type T = int }
  {
   return $inv_box->unwrap()->get(); // ERROR: expected T2 but got int
  }
}
