<?hh

interface Constraint<T as num> {}

abstract class InvalidConstraint {
  abstract const type T as Vector<Constraint<string>>;
}
