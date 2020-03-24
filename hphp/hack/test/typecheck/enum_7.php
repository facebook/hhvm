<?hh // partial

abstract class Enum {
  abstract const type TInner;
}

// Should fail because the bool const is bogus
class Bar {
  const bool BAZ = true;
}

// Should fail because the bool const is bogus
class Foo extends Enum {
  const type TInner = mixed;
  const int FOO = 0;
  const string BAR = "lol";
  const BAZ = Bar::BAZ;
}
