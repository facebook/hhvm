<?hh
//
<<file: __EnableUnstableFeatures('named_parameters')>>

class InlineDuplicateNamed {
  private function inline_me(named int $first, named int $second): int {
    return $first + $second;
  }

  public function caller(): int {
    return $this->/*range-start*/inline_me/*range-end*/(first=1, first=2);
  }
}
