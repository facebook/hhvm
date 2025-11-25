<?hh
<<file:__EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>

function test_immediately_called_lambda(): void {
  ((named int $x, named string $y) ==> $x)(x=42, y="hello");
  ((named int $a, named string $b) ==> $a + 5)(a=10, b="test");
}

function test_lambda_stored_in_variable(): void {
  $f = ((named int $x, named string $y) ==> $x);
  $f(x=42, y="hello");
}
