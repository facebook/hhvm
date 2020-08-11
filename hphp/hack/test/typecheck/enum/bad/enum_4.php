<?hh // strict

abstract class Enum<T> {
  abstract const type TInner;
}

// Should fail because bool constants not allowed
class Foo extends Enum {
  const type TInner = bool;
  const bool FOO = true;
}
