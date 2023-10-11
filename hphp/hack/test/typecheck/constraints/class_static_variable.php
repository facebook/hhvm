<?hh

interface Constraint<T as num> {}

class InvalidConstraint {
  public static ?(int, Constraint<string>) $x = null;
}
