<?hh

interface Constraint<T as num> {}

class InvalidConstraint {
  public function foo<T as Constraint<mixed>>(T $c): void {}
}
