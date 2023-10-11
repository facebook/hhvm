<?hh

interface Constraint<T as num> {}

newtype NewConstraint as mixed = Constraint<mixed>;
