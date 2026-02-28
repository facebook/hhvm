<?hh

abstract class Enum {
  abstract const type TInner;
}

class E extends Enum {
  const type TInner = nonnull;
  const FOO = 'foo';
  const BAR = 42;
  const BAZ = true;
}
