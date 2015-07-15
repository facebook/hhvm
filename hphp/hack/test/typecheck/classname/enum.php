<?hh

abstract class Enum<T> {}

abstract class C {}
class C1 extends C {}
class C2 extends C {}

class X extends Enum<classname<C>> {
  const ONE = C1::class;
  const TWO = C2::class;
}
