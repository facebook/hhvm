<?hh // strict

abstract class Enum<T> {
}

// Should fail because bool constants not allowed
class Foo extends Enum<bool> {
  const bool FOO = true;
}
