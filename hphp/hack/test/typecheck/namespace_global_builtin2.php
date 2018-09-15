<?hh // strict

namespace Test;

function invariant_violation(): int {
  return 10;
}

function f(): string {
  invariant_violation('auto-import');
}

function g(): int {
  return \Test\invariant_violation();
}

interface Traversable<T> {}

// Refers to Traversable introduced into Test
class Vector implements Traversable<int> {}

function h(): Traversable<int> {
  return new \Test\Vector();
}
