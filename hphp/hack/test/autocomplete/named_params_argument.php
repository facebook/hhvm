<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

final class C {
  public function testNamedParameterAutocomplete(): void {
    $this->namedParameterAutocompleteRequiredBeforeOptionalExample(
      zz_named_used='',
      zz_named_AUTO332,
    );
  }

  private function namedParameterAutocompleteRequiredBeforeOptionalExample(
    named string $zz_named_opt_z = '',
    named int $zz_named_req_z,
    named bool $zz_named_opt_a = false,
    named float $zz_named_req_a,
    named string $zz_named_used = '',
  ): void {}
}
