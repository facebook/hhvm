<?hh

function where_constraint_violation(Vector<?string> $xs): void {
  $xs->toSet();
}
