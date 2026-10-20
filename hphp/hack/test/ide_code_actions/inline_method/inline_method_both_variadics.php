<?hh
//
<<file: __EnableUnstableFeatures('named_parameters')>>

class InlineBothVariadics {
  private function inline_me(int ...$numbers, named string...): int {
    return count($numbers);
  }

  public function caller(): int {
    return $this->/*range-start*/inline_me/*range-end*/(1, label='hello', 2, more='world');
  }
}
//
