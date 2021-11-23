<?hh

// We currently parse FALLTHROUGH comments as Fallthrough statements only
// within switch statements, if we parsed them as Fallthrough statements
// indiscriminately and treat them in the typechecker as we do today (by
// moving the Next continuation to Fallthrough continuation), the following
// program would fail. This test makes sure we catch this if such a change is
// ever made.
function ko(int $x): void {
  // FALLTHROUGH
  hh_expect<int>($x);
}
