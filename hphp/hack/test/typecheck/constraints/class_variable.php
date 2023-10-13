<?hh

interface Constraint<T as num> {}

class InvalidConstraint {
  public ?(function(): Constraint<string>) $x = null;
}
