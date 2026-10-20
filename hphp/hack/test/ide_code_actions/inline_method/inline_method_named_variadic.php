<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class InlineNamedVariadic {
  private function inline_me(int $first, named string $label, named string...): string {
    return $label.$first;
  }

  private function observe(string $x): string {
    return $x;
  }

  public function caller(): string {
    return $this->/*range-start*/inline_me/*range-end*/(
      extra=$this->observe('first'),
      2,
      label=$this->observe('label'),
      more=$this->observe('last'),
    );
  }
}
//
