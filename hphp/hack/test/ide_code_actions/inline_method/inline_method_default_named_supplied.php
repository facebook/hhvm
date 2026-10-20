<?hh
//
<<file: __EnableUnstableFeatures('named_parameters')>>

class InlineDefaultNamedSupplied {
  private function inline_me(int $first = 1, named int $choice = 2, int $last = 3): int {
    return $first + $choice + $last;
  }

  public function caller(): int {
    return $this->/*range-start*/inline_me/*range-end*/(choice=5, 6);
  }
}
//
