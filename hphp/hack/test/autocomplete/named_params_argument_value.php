<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class C {
  public function example(named int $zz_named_required): void {}
}

function test_named_parameter_value_autocomplete(
  C $example,
): void {
  $example->example(zz_named_required=zz_named_AUTO332);
}
