<?hh

interface IDupe<+T> {}

interface I {}

interface IChild
  extends I1, J1 {}

interface I1 extends IEntBase {
  abstract const type T as I;
}

interface IEntBase extends IDupe<this::T> {
  abstract const type T;
}

interface J1 extends IDupe<I> {}
