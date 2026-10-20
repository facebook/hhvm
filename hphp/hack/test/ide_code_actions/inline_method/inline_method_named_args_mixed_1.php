<?hh
//
<<file: __EnableUnstableFeatures('named_parameters')>>

class InlineMixedArgs {
  private function inline_me(string $a, named int $b, named string $c): string {
    return $a.$b.$c;
  }
  public function caller(): string {
    return $this->/*range-start*/inline_me/*range-end*/("x", c="y", b=3);
  }
}
//
