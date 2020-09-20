<?hh // partial

abstract class Enum {
  abstract const type TInner;
}

abstract class C {}
class C1 extends C {}
class C2 extends C {}

class X extends Enum {
  const type TInner = classname<C>;
  const ONE = C1::class;
  const TWO = C2::class;
}
