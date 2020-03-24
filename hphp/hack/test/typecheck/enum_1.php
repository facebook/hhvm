<?hh // strict

abstract class Enum {
  abstract const type TInner;
}

// Should fail because it doesn't match
class Foo extends Enum {
  const type TInner = int;
  const string FOO = "foo";
}
