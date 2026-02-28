<?hh
<<file: __EnableUnstableFeatures('named_parameters_use', 'named_parameters')>>

function test_named_invariant(): void {
  // Error: invariant does not accept named arguments
  invariant(cond = true, "message");
}

function test_named_invariant_violation(): void {
  // Error: invariant_violation does not accept named arguments
  invariant_violation(format = "error");
}

function test_named_exit(): void {
  // Error: exit does not accept named arguments
  exit(status = 0);
}
