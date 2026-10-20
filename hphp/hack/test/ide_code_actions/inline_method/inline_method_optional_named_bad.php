<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class InlineOptionalNamed {
  private function inline_me(optional named int $x): int {
    return $x;
  }

  public function caller(): int {
    return $this->/*range-start*/inline_me/*range-end*/();
  }
}
