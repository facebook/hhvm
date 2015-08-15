<?hh // strict

interface Constraint<T as num> {}

// Not a valid constraint since T is unbounded type parameter
class InvalidConstraint<T, Tc as Constraint<T>> {}
