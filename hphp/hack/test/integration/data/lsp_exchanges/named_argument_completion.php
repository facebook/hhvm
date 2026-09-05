<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class NamedArgumentCompletion {
  public function example(
    named string $zz_named_opt_z = '',
    named int $zz_named_req_z,
    named bool $zz_named_opt_a = false,
    named float $zz_named_req_a,
    named string $zz_named_used = '',
  ): void {}
}

function named_argument_completion_area(
  NamedArgumentCompletion $example,
): void {

}
