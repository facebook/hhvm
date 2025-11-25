<?hh
<<file:__EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>

function test_type_mismatch(): void {
  // Type mismatch: passing string where int is expected
  $result = ((named int $x, named string $y) ==> $x)(x="wrong", y="hello");
}

function test_wrong_parameter_name(): void {
  // Wrong parameter name: using 'z' instead of 'x'
  $result = ((named int $x, named string $y) ==> $x)(z=42, y="hello");
}

function test_missing_named_parameter(): void {
  // Missing required named parameter 'y'
  $result = ((named int $x, named string $y) ==> $x)(x=42);
}

function test_extra_parameter(): void {
  // Extra parameter 'z' that doesn't exist
  $result = ((named int $x, named string $y) ==> $x)(x=42, y="hello", z=100);
}
