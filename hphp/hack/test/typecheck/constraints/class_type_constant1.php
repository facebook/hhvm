<?hh

interface Constraint<T as num> {}

class InvalidConstraint {
  const type T = varray<Constraint<string>>;
}
