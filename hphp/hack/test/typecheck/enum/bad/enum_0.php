<?hh

abstract class Enum<T> {
}

// Should fail because it doesn't match
class Foo extends Enum<int> {
  const string FOO = "foo";
}
