<?hh // strict

class Foo<T> extends Exception {}

function f(mixed $x): Foo<int> {
  try {
    throw new Foo('');
  } catch (Foo $y) {
    throw $y;
  }
}
