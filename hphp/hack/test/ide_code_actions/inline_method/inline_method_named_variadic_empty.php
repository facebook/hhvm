<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class InlineNamedVariadicEmpty {
  private function inline_me(named int...): int {
    return 1;
  }

  public function caller(): int {
    return $this->/*range-start*/inline_me/*range-end*/();
  }
}
//
