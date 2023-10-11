<?hh

interface Constraint<T as num> {}

type AliasConstraint<T> = Constraint<T>;

// Test that we check constraints after expand the type alias
function invalid(AliasConstraint<mixed> $x): void {}
