<?hh

interface Constraint<T as num> {}

// We do not need to constrain the type parameter for type
// definitions since they cannot put constraints on their type parameters
type AliasConstraint<T> = Constraint<T>;

newtype NewConstraint<T as num> as Constraint<T> = AliasConstraint<T>;
