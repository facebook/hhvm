<?hh

interface I1 {}
interface I2 extends I1 {}

abstract class A1<T as I2> {}
abstract class A2 extends A1<this::TI> {
  const type TI = I1;
}
