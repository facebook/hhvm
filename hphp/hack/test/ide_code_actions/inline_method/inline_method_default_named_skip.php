<?hh
//
<<file: __EnableUnstableFeatures('named_parameters')>>

class InlineDefaultNamedSkip {
  private function inline_me(int $first, named int $choice = 2, int $last = 3): int {
    return $first + $choice + $last;
  }

  public function caller(): int {
    return $this->/*range-start*/inline_me/*range-end*/(1, 4);
  }
}
//
