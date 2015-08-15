<?hh // strict

interface Constraint<T as num> {}

class InvalidConstraint {
  public function foo<T>(Constraint<T> $c): void {}
}
