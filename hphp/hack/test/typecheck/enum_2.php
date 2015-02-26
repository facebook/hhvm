<?hh // strict

abstract class Enum<T> {}

class Foo extends Enum<mixed> {
  const int FOO = 0;
  const string BAR = "lol";
}

class Bar extends Enum<arraykey> {
  const int FOO = 0;
  const string BAR = "lol";
}
