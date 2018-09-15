<?hh // strict

abstract class Enum<T> {}

class E extends Enum<nonnull> {
  const FOO = 'foo';
  const BAR = 42;
  const BAZ = true;
}
