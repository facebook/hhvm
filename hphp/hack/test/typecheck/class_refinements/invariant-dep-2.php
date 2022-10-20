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
class ConcreteOuterGood extends Outer<self::TBox> {
  const type TBT = int;
  const type TBox = ConcreteIntBox;

  <<__Override>>
  public function boxGet(
    Invariant<self::TBox> $inv_box
  ): int
  // Override OK, since we know that:
  //   self::TBox (= ConcreteIntBox) <: Box with { type T = T }
  // so it must be that T = int
  {
    return $inv_box->unwrap()->get();
  }
}

class ConcreteOuterBad extends Outer<self::TBox> {
  const type TBT = int;
  const type TBox = ConcreteIntBox;

  <<__Override>>
  public function boxGet<T2 as int>(
    Invariant<self::TBox> $inv_box
  ): T2
  {
    return $inv_box->unwrap()->get();
    // ERROR: expected T2 but got int
    // As expected, since T2 could be, for
    // example, 'nothing'.
  }
}
