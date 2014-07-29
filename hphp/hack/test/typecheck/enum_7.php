<?hh

abstract class Enum<T> {}

// Should fail because the bool const is bogus
class Bar {
  const bool BAZ = true;
}

// Should fail because the bool const is bogus
class Foo extends Enum<mixed> {
  const int FOO = 0;
  const string BAR = "lol";
  const BAZ = Bar::BAZ;
}
