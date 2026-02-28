<?hh

abstract final class Foo extends Exception {}

function f(mixed $x): void {
  try {
    throw new Exception('');
  } catch (Foo $y) {
  }
}
