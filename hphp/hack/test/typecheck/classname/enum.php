<?hh

abstract class Enum {
  abstract const type TInner;
}

abstract class C {}
class C1 extends C {}
class C2 extends C {}

class X extends Enum {
  const type TInner = classname<C>;
  const classname<C1> ONE = C1::class;
  const classname<C2> TWO = C2::class;
}
