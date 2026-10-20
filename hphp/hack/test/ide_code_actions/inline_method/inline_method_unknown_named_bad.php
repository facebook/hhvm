<?hh
//
<<file: __EnableUnstableFeatures('named_parameters')>>

class InlineUnknownNamed {
  private function inline_me(int $first, named int $known): int {
    return $first + $known;
  }

  public function caller(): int {
    return $this->/*range-start*/inline_me/*range-end*/(1, wrong=2);
  }
}
